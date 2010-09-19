//
//  $Id: telnetd.h 345 2008-11-11 03:05:06Z jcw $
//  $Revision: 345 $
//  $Author: jcw $
//  $Date: 2008-11-10 22:05:06 -0500 (Mon, 10 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/apps/telnetd/telnetd.h $
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
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * $Id: telnetd.h 345 2008-11-11 03:05:06Z jcw $
 *
 */
#ifndef __TELNETD_H__
#define __TELNETD_H__

#include "FreeRTOS.h"

#include "../../uip/uipopt.h"

//
//
//
#ifndef TELNETD_CONF_LINELEN
#define TELNETD_CONF_LINELEN 40
#endif
#ifndef TELNETD_CONF_NUMLINES
#define TELNETD_CONF_NUMLINES 16
#endif

//
//
//
typedef enum
{
  TELNETDSTATE_NORMAL = 0,
  TELNETDSTATE_IAC,
  TELNETDSTATE_WILL,
  TELNETDSTATE_WONT,
  TELNETDSTATE_DO,
  TELNETDSTATE_DONT,
  TELNETDSTATE_CLOSE,
}
telnetdState_e;

//
//
//
typedef struct telnetdState_s
{
  u16_t numsent;
  telnetdState_e state;
  struct uip_conn *conn;
}
telnetdState_t;

typedef telnetdState_t uip_tcp_appstate_telnetd;

//
//
//
portBASE_TYPE telnetdPutChar (portCHAR c);
void telnetdDisconnect (void);
int telnetd_init (void);
void telnetd_appcall (void);

#ifndef UIP_APPCALL
#define UIP_APPCALL telnetd_appcall
#endif

#endif /* __TELNETD_H__ */
