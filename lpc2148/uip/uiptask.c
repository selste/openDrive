//
//  $Id: uiptask.c 344 2008-11-11 03:04:49Z jcw $
//  $Revision: 344 $
//  $Author: jcw $
//  $Date: 2008-11-10 22:04:49 -0500 (Mon, 10 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/uiptask.c $
//

/*
 * Modified from an original work that is Copyright (c) 2001-2003, Adam Dunkels.
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
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uiptask.c 344 2008-11-11 03:04:49Z jcw $
 *
 */

//
//  Standard includes 
// 
#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "../rtc/rtc.h"
#include "../main.h"

#undef HTONS
#include "hardware/enc28j60.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "apps/webserver/httpd.h"
#include "apps/telnetd/telnetd.h"
#include "apps/dhcpc/dhcpc.h"
#include "apps/sntp/sntp.h"
#include "uiptask.h"

//
//  The start of the uIP buffer, which will contain the frame headers
//
#define pucUIP_Buffer ((struct uip_eth_hdr *) &uip_buf [0])

//
//  uIP update frequencies
//
#define RT_CLOCK_SECOND   (configTICK_RATE_HZ)
#define uipARP_FREQUENCY  (10)
#define uipMAX_BLOCK_TIME (RT_CLOCK_SECOND / 10)

//
//  If CFG_UIP is not defined, we still compile the code, it's just not included.
//  Define TASKHANDLE_UIP so we compile.  Normally this is defined in main.h,
//  but it's #ifndef'ed out so that we don't allocate a task handle slot for it
//  when uIP support isn't included.
//
#ifndef CFG_UIP
#define TASKHANDLE_UIP 0
#endif

//
//
//
u8_t uip_buf [UIP_BUFSIZE + 8] __attribute__ ((aligned (4)));

static int useDHCP = TRUE;
static xQueueHandle xFastPollQueue = NULL; 

//
//
//
static portTASK_FUNCTION (vUIPTask, pvParameters)
{
  portBASE_TYPE xARPTimer;
  volatile portBASE_TYPE *errFlag = (portBASE_TYPE *) pvParameters;
  static volatile portTickType xStartTime, xCurrentTime;

  if (!xFastPollQueue)
    xFastPollQueue = xQueueCreate (1, (unsigned portBASE_TYPE) sizeof (struct uip_conn *));

  //
  //  Initialize the uIP TCP/IP stack
  //
  uip_init ();
  uip_arp_init ();

  //
  //  Initialize the Ethernet controller hardware
  //
  if (!enc28j60Init ())
  {
    if (errFlag)
      *errFlag = 1;

    while (1)
      vTaskDelay (1000 / portTICK_RATE_MS);
  }

  if (errFlag)
    *errFlag = 0;

  //
  //  Either we've got an address, or we need to request one
  //
  if (useDHCP)
  {
#ifdef CFG_DHCP
    dhcpc_init (uip_ethaddr.addr, sizeof (uip_ethaddr.addr));
    dhcpc_request ();
#else
    printf ("DHCP support not enabled\n");
#endif
  }
  else
  {
#ifdef CFG_HTTPD
    httpd_init ();
#endif
#ifdef CFG_TELNETD
    telnetd_init ();
#endif
  }

  //
  //  Initialise the local timers
  //
  xStartTime = xTaskGetTickCount ();
  xARPTimer = 0;

  //
  //  The big loop
  //
  while (1)
  {
    /* enc28j60WaitForData() returns pdTRUE if we were woken by an 
       interrupt, otherwise pdFALSE if the timer expired.  If pdTRUE,
       then the ENC28J60 has data ready for us. */
    if (enc28j60WaitForData (uipMAX_BLOCK_TIME) == pdTRUE)
    {
      /* Let the network device driver read an entire IP packet
         into the uip_buf. If it returns > 0, there is a packet in the
         uip_buf buffer. */
      if ((uip_len = enc28j60Receive ()) > 0)
      {
        /* A packet is present in the uIP buffer. We call the
           appropriate ARP functions depending on what kind of packet we
           have received. If the packet is an IP packet, we should call
           uip_input() as well. */
        if (pucUIP_Buffer->type == htons (UIP_ETHTYPE_IP))
        {
          uip_arp_ipin ();
          uip_input ();

          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */
          if (uip_len > 0)
          {
            uip_arp_out ();
            enc28j60Send ();
          }
        }
        else if (pucUIP_Buffer->type == htons (UIP_ETHTYPE_ARP))
        {
          uip_arp_arpin ();

          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */  
          if (uip_len > 0)
            enc28j60Send ();
        }
      }
    }
    else
    {
      struct uip_conn *conn;

      //
      //  If there's data in the short circuit queue, it means that
      //  uipFastPoll() was called with a connection number, and
      //  we need to poll that connection.
      //
      if (xFastPollQueue && (xQueueReceive (xFastPollQueue, &conn, 0) == pdTRUE))
      {
        uip_poll_conn (conn);

        if (uip_len > 0)
        {
          uip_arp_out ();
          enc28j60Send ();
        }
      }

      /* The poll function returned 0, so no packet was
         received. Instead we check if it is time that we do the
         periodic processing. */
      xCurrentTime = xTaskGetTickCount ();

      if ((xCurrentTime - xStartTime) >= RT_CLOCK_SECOND)
      {
        portBASE_TYPE i;

        /* Reset the timer. */
        xStartTime = xCurrentTime;

        /* Periodic check of all connections. */
        for (i = 0; i < UIP_CONNS; i++)
        {
          uip_periodic (i);

          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */          
          if (uip_len > 0)
          {
            uip_arp_out ();
            enc28j60Send ();
          }
        }

#if UIP_UDP
        for (i = 0; i < UIP_UDP_CONNS; i++)
        {
          uip_udp_periodic (i);

          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */
          if (uip_len > 0)
          {
            uip_arp_out ();
            enc28j60Send ();
          }
        }
#endif /* UIP_UDP */

        /* Periodically call the ARP timer function. */
        if (++xARPTimer == uipARP_FREQUENCY)
        { 
          uip_arp_timer ();
          xARPTimer = 0;
        }
      }
    }
  }
}

//
//
//
void uipFastPoll (struct uip_conn *conn)
{
  if (xFastPollQueue)
    xQueueSend (xFastPollQueue, &conn, portMAX_DELAY);
}

//
//
//
portBASE_TYPE uipIsRunning (void)
{
  return taskHandles [TASKHANDLE_UIP] ? 1 : 0;
}

uipState_e uipStart (int doDHCP)
{
  portBASE_TYPE errFlag = -1;
  volatile portTickType xTicks;

  if (uipIsRunning ())
    return UIPSTATE_ALREADYRUNNING;
  
  useDHCP = doDHCP;

  xTaskCreate (vUIPTask, (const signed portCHAR * const) "uIP", 768, &errFlag, (tskIDLE_PRIORITY + 1), &taskHandles [TASKHANDLE_UIP]);

  xTicks = xTaskGetTickCount ();

  while ((volatile portBASE_TYPE) errFlag == -1)
  {
    vTaskDelay (100 / portTICK_RATE_MS);

    if ((xTaskGetTickCount () - xTicks) > (5000 / portTICK_RATE_MS))
    {
      uipStop ();
      return UIPSTATE_TASKDIDNTSTART;
    }
  }

  if (errFlag == 1)
  {
    uipStop ();
    return UIPSTATE_ENCJ28C60ERR;
  }

  return UIPSTATE_OK;
}

uipState_e uipStop (void)
{
  if (!uipIsRunning ())
    return UIPSTATE_NOTRUNNING;

  vTaskDelete (taskHandles [TASKHANDLE_UIP]);
  taskHandles [TASKHANDLE_UIP] = NULL;
  enc28j60Deinit ();
  
  return UIPSTATE_OK;
}

void dispatch_tcp_appcall (void)
{
#ifdef CFG_HTTPD
  if (uip_conn->lport == HTONS (80))
    httpd_appcall ();
#endif
#ifdef CFG_TELNETD
  if (uip_conn->lport == HTONS (23))
    telnetd_appcall ();
#endif
}

#if UIP_UDP
void dispatch_udp_appcall (void)
{
#ifdef CFG_SNTP
  if (uip_udp_conn->rport == HTONS (123))
    sntp_appcall ();
  else
#endif
#ifdef CFG_DHCP
  if (uip_udp_conn->rport == HTONS (DHCPC_SERVER_PORT))
    dhcpc_appcall ();
#endif
}
#endif


#ifdef CFG_AUTO_SNTP
//
//
//
static void uipAutoSNTPTimeSynced (time_t *epochSeconds)
{
  if (*epochSeconds)
  {
    n32_t to;
      
    uip_gettimeoffset (&to);
    *epochSeconds += to;
    rtcSetEpochSeconds (*epochSeconds);
  }
}

static void uipAutoSNTP (void)
{
  uip_ipaddr_t addr;

  uip_getsntpaddr (&addr);

  if (!uip_iszeroaddr (&addr))
    sntpSync (&addr, uipAutoSNTPTimeSynced);
}
#endif

void dhcpc_configured (const dhcpcState_t *s)
{
#ifdef CFG_DHCP
  if (!s->ipaddr [0] && !s->ipaddr [1])
  {
    printf ("Can't get address via DHCP and no static address configured, stopping uIP task\n");
    uipStop ();
  }
  else 
  {
    printf ("\nIP address via DHCP is %d.%d.%d.%d\n" PROMPT, uip_ipaddr1 (s->ipaddr), uip_ipaddr2 (s->ipaddr), uip_ipaddr3 (s->ipaddr), uip_ipaddr4 (s->ipaddr));
    fflush (stdout);
    uip_sethostaddr (s->ipaddr);
    uip_setnetmask (s->netmask);
    uip_setdraddr (s->default_router);
    uip_setsntpaddr (s->sntpaddr);
    uip_settimeoffset (&s->timeoffset);

#ifdef CFG_HTTPD
    httpd_init ();
#endif
#ifdef CFG_TELNETD
    telnetd_init ();
#endif
#ifdef CFG_AUTO_SNTP
    uipAutoSNTP ();
#endif
  }
#else
  s = s;
#endif
}
