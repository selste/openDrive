//
//  $Id: telnetd.c 345 2008-11-11 03:05:06Z jcw $
//  $Revision: 345 $
//  $Author: jcw $
//  $Date: 2008-11-10 22:05:06 -0500 (Mon, 10 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/apps/telnetd/telnetd.c $
//

/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * $Id: telnetd.c 345 2008-11-11 03:05:06Z jcw $
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <string.h>

#if defined CFG_CONSOLE_USB
#include "../../usbser/usbser.h"
#elif defined CFG_CONSOLE_UART0
#include "../../uart/uart0.h"
#elif defined CFG_CONSOLE_UART1
#include "../../uart/uart1.h"
#else
#error "Eeek!  No console devices defined!"
#endif

#include "../../../main.h"
#include "../../uiptask.h"
#include "../../uip/uip.h"
#include "../../lib/memb.h"
#include "telnetd.h"

//
//
//
#define ISO_nl 0x0a
#define ISO_cr 0x0d

#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254

//
//
//
typedef struct telnetdBuf_s
{
  char bufmem [TELNETD_CONF_NUMLINES * TELNETD_CONF_LINELEN];
  int ptr;
  int size;
}
telnetdBuf_t;

//
//
//
static telnetdState_t *telnetdState = NULL;
static telnetdBuf_t telnetdBuf;
static xQueueHandle telnetdQueueTx;
static xQueueHandle consoleQueue = NULL;

//
//  Useful for debugging, as it disables printing to the telnet client
//
#if 0
#include <stdio.h>
#include <stdarg.h>

static void tprintf (char *fmt, ...)
{
  va_list ap;
  telnetdState_t *telnetdStateTemp = telnetdState;

  telnetdState = NULL;
  va_start (ap, fmt);
  vprintf (fmt, ap);
  va_end (ap);
  telnetdState = telnetdStateTemp;
}
#endif

//
//
//
static void telnetdBufferInit (telnetdBuf_t *buf)
{ 
  buf->ptr = 0;
  buf->size = TELNETD_CONF_NUMLINES * TELNETD_CONF_LINELEN;
}

//
//
//
static int telnetdBufferAppend (telnetdBuf_t *buf, const char *data, int len)
{ 
  int copylen;

  copylen = MIN (len, buf->size - buf->ptr);
  memcpy (&buf->bufmem [buf->ptr], data, copylen);
  buf->ptr += copylen;

  return copylen;
}

//
//
//
static void telnetBufferCopyTo (telnetdBuf_t *buf, char *to, int len)
{
  if (len)
    memcpy (to, &buf->bufmem [0], len);
}

//
//
//
static void telnetBufferPop (telnetdBuf_t *buf, int len)
{
  int poplen;

  poplen = MIN (len, buf->ptr);
  memcpy (&buf->bufmem [0], &buf->bufmem [poplen], buf->ptr - poplen);
  buf->ptr -= poplen;
}

//
//
//
static int telnetBufferLength (telnetdBuf_t *buf)
{
  return buf->ptr;
}

//
//
//
static int telnetdBufferHasSpace (telnetdBuf_t *buf)
{
  return buf->size - buf->ptr;
}

//
//
//
static void telnetdAcked (void)
{
  if (telnetdState)
    telnetBufferPop (&telnetdBuf, telnetdState->numsent);
}

//
//
//
static void telnetdSendData (void)
{
  if (telnetdState)
  {
    int len;

    len = MIN (telnetBufferLength (&telnetdBuf), uip_mss ());
    telnetBufferCopyTo (&telnetdBuf, uip_appdata, len);
    uip_send (uip_appdata, len);
    telnetdState->numsent = len;
  }
}

//
//
//
static int telnetdOpen (void)
{
  uip_unlisten (HTONS (23));

  telnetdBufferInit (&telnetdBuf);
  telnetdState = (telnetdState_t *) &(uip_conn->appstate);
  telnetdState->state = TELNETDSTATE_NORMAL;
  telnetdState->conn = uip_conn;
  telnetdBufferAppend (&telnetdBuf, PROMPT, strlen (PROMPT));

  return 1;
}

//
//  When a telnet close is requested, we need to read the Tx queue until it
//  returns empty.  This prevents us from hanging on trying to send data to a
//  closed socket.
//
static void telnetdClose (void)
{
  portCHAR c;

  while (xQueueReceive (telnetdQueueTx, &c, 0) == pdTRUE)
    ;

  telnetdState = NULL;

  uip_listen (HTONS (23));
}

//
//  Send a character to the monitor queue to be processed.  CRs are stripped
//  because telnet always sends CR/LF, and we only care about LF.  Post the
//  received character into the monitor queue.
//
static void telnetdNewRxChar (u8_t c)
{
  if (!c || (c == ISO_cr) || !consoleQueue)
    return;

  xQueueSend (consoleQueue, &c, portMAX_DELAY);
}

//
//  See if the monitor has placed any data in the Tx queue, and if so,
//  stuff it into the outgoing telnet buffer.
//
static void telnetdNewTxData (void)
{
  portCHAR c;

  while (telnetdBufferHasSpace (&telnetdBuf) && (xQueueReceive (telnetdQueueTx, &c, 0) == pdTRUE))
    telnetdBufferAppend (&telnetdBuf, (char *) &c, sizeof (c));
}

//
//  Telnet will/wont/do/dont formatting
//
static void telnetdSendOpt (u8_t option, u8_t value)
{
  char line [4];

  line [0] = TELNET_IAC;
  line [1] = option;
  line [2] = value;
  line [3] = 0;

  telnetdBufferAppend (&telnetdBuf, line, 4);
}

//
//  New data has been received from the client
//
static void telnetdNewRxData (void)
{
  if (telnetdState)
  {
    u16_t len;
    u8_t c;
    u8_t *dataptr;

    len = uip_datalen ();
    dataptr = (u8_t *) uip_appdata;

    while (len > 0)
    {
      c = *dataptr++;
      len--;

      switch (telnetdState->state) 
      {
        case TELNETDSTATE_NORMAL :
          {
            if (c == TELNET_IAC) 
              telnetdState->state = TELNETDSTATE_IAC;
            else
              telnetdNewRxChar (c);
          }
          break;

        case TELNETDSTATE_IAC :
          {
            if (c == TELNET_IAC) 
            {
              telnetdNewRxChar (c);
              telnetdState->state = TELNETDSTATE_NORMAL;
            } 
            else 
            {
              switch (c) 
              {
                case TELNET_WILL :
                  telnetdState->state = TELNETDSTATE_WILL;
                  break;

                case TELNET_WONT :
                  telnetdState->state = TELNETDSTATE_WONT;
                  break;

                case TELNET_DO :
                  telnetdState->state = TELNETDSTATE_DO;
                  break;

                case TELNET_DONT :
                  telnetdState->state = TELNETDSTATE_DONT;
                  break;

                default :
                  telnetdState->state = TELNETDSTATE_NORMAL;
                  break;
              }
            }
          }
          break;

        case TELNETDSTATE_WILL : /* Reply with a DONT */
          {
            telnetdSendOpt (TELNET_DONT, c);
            telnetdState->state = TELNETDSTATE_NORMAL;
          }
          break;

        case TELNETDSTATE_WONT : /* Reply with a DONT */
          {
            telnetdSendOpt (TELNET_DONT, c);
            telnetdState->state = TELNETDSTATE_NORMAL;
          }
          break;

        case TELNETDSTATE_DO : /* Reply with a WONT */
          {
            telnetdSendOpt (TELNET_WONT, c);
            telnetdState->state = TELNETDSTATE_NORMAL;
          }
          break;

        case TELNETDSTATE_DONT : /* Reply with a WONT */
          {
            telnetdSendOpt (TELNET_WONT, c);
            telnetdState->state = TELNETDSTATE_NORMAL;
          }
          break;

        case TELNETDSTATE_CLOSE :
          break;
      }
    }
  }
}

//
//  Character from monitor task, to be sent to telnet client.  Return pdFALSE
//  if telnet session not open.  Return pdTRUE if session open and character
//  posted.  If the queue doesn't clear for 5 seconds, close the session and
//  return errQUEUE_FULL
//
portBASE_TYPE telnetdPutChar (portCHAR c)
{
  portBASE_TYPE r = pdFALSE;

  if (telnetdState && (telnetdState->state == TELNETDSTATE_NORMAL))
  {
    if ((r = xQueueSend (telnetdQueueTx, &c, 0)) != pdTRUE)
    {
      uipFastPoll (telnetdState->conn);

      if ((r = xQueueSend (telnetdQueueTx, &c, 5000 / portTICK_RATE_MS)) != pdTRUE)
        telnetdClose ();
    }
  }

  return r;
}

//
//
//
void telnetdDisconnect (void)
{
  if (telnetdState)
    telnetdState->state = TELNETDSTATE_CLOSE;
}

//
//
//
int telnetd_init (void)
{
  if (!telnetdQueueTx)
    if (!(telnetdQueueTx = xQueueCreate (256, (unsigned portBASE_TYPE) sizeof (signed portCHAR))))
      return 0;

  uip_listen (HTONS (23));
  telnetdBufferInit (&telnetdBuf);
  telnetdState = NULL;

#if defined CFG_CONSOLE_USB
  usbserGetRxQueue (&consoleQueue);
#elif defined CFG_CONSOLE_UART0
  uart0GetRxQueue (&consoleQueue);
#elif defined CFG_CONSOLE_UART1
  uart1GetRxQueue (&consoleQueue);
#else
#error "Eeek!  No console devices defined!"
#endif

  return 1;
}

//
//
//
void telnetd_appcall (void)
{
  if (uip_connected ()) 
    if (!telnetdOpen ())
      return;

  if (telnetdState && (telnetdState->state == TELNETDSTATE_CLOSE))
    uip_close ();

  if (uip_closed () || uip_aborted () || uip_timedout ())
    telnetdClose ();

  if (uip_acked ())
    telnetdAcked ();

  if (uip_newdata ())
    telnetdNewRxData ();

  if (uip_rexmit () || uip_newdata () || uip_acked () || uip_connected ())
    telnetdSendData ();
  else if (uip_poll ())  
  {
    telnetdNewTxData ();
    telnetdSendData ();
  }
}
