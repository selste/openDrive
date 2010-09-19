//
//  $Id: dhcpc.c 278 2008-11-02 19:49:40Z jcw $
//  $Revision: 278 $
//  $Author: jcw $
//  $Date: 2008-11-02 14:49:40 -0500 (Sun, 02 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/apps/dhcpc/dhcpc.c $
//

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * @(#)$Id: dhcpc.c 278 2008-11-02 19:49:40Z jcw $
 */

#include <stdio.h>
#include <string.h>

#include "../../uip/uip.h"
#include "../../uip/uip_timer.h"
#include "../../uip/pt.h"
#include "dhcpc.h"

#define STATE_INITIAL         0
#define STATE_SENDING         1
#define STATE_OFFER_RECEIVED  2
#define STATE_CONFIG_RECEIVED 3

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_TIME_OFFSET   2
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_NTP_SERVER   42
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255

//
//
//
typedef struct __attribute ((packed)) dhcpMsg_s
{
  u8_t  op;
  u8_t  htype;
  u8_t  hlen;
  u8_t  hops;
  u8_t  xid [4];
  u16_t secs;
  u16_t flags;
  u8_t  ciaddr [4];
  u8_t  yiaddr [4];
  u8_t  siaddr [4];
  u8_t  giaddr [4];
  u8_t  chaddr [16];
#ifndef UIP_CONF_DHCP_LIGHT
  u8_t  sname [64];
  u8_t  file [128];
#endif
  u8_t  options [312];
}
__attribute__ ((packed)) dhcpMsg_t;

//
//
//
static dhcpcState_t *dhcpcState;
static const u8_t xid [4] = {0xad, 0xde, 0x12, 0x23};
static const u8_t magic_cookie [4] = {99, 130, 83, 99};

//
//
//
static u8_t *add_msg_type (u8_t *optptr, u8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;

  return optptr;
}

//
//
//
static u8_t *add_server_id (u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy (optptr, dhcpcState->serverid, 4);

  return optptr + 4;
}

//
//
//
static u8_t *add_req_ipaddr (u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_IPADDR;
  *optptr++ = 4;
  memcpy (optptr, dhcpcState->ipaddr, 4);

  return optptr + 4;
}

//
//
//
static u8_t *add_req_options (u8_t *optptr)
{
  u8_t *listStart;
  *optptr++ = DHCP_OPTION_REQ_LIST;
  listStart = optptr++;
  *optptr++ = DHCP_OPTION_SUBNET_MASK;
  *optptr++ = DHCP_OPTION_ROUTER;
  *optptr++ = DHCP_OPTION_DNS_SERVER;
  *optptr++ = DHCP_OPTION_NTP_SERVER;
  *optptr++ = DHCP_OPTION_TIME_OFFSET;
  *listStart = (optptr - listStart) - 1;

  return optptr;
}

//
//
//
static u8_t *add_end (u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;

  return optptr;
}

//
//
//
static void create_msg (register dhcpMsg_t *m)
{
  m->op = DHCP_REQUEST;
  m->htype = DHCP_HTYPE_ETHERNET;
  m->hlen = dhcpcState->mac_len;
  m->hops = 0;
  memcpy (m->xid, xid, sizeof (m->xid));
  m->secs = 0;
  m->flags = HTONS (BOOTP_BROADCAST);
  memcpy (m->ciaddr, uip_hostaddr, sizeof (m->ciaddr));
  memset (m->yiaddr, 0, sizeof (m->yiaddr));
  memset (m->siaddr, 0, sizeof (m->siaddr));
  memset (m->giaddr, 0, sizeof (m->giaddr));
  memcpy (m->chaddr, dhcpcState->mac_addr, dhcpcState->mac_len);
  memset (&m->chaddr [dhcpcState->mac_len], 0, sizeof (m->chaddr) - dhcpcState->mac_len);
#ifndef UIP_CONF_DHCP_LIGHT
  memset (m->sname, 0, sizeof (m->sname));
  memset (m->file, 0, sizeof (m->file));
#endif

  memcpy (m->options, magic_cookie, sizeof (magic_cookie));
}

//
//
//
static void send_discover (void)
{
  u8_t *end;
  dhcpMsg_t *m = (dhcpMsg_t *) uip_appdata;

  create_msg (m);

  end = add_msg_type (&m->options [4], DHCPDISCOVER);
  end = add_req_options (end);
  end = add_end (end);

  uip_send (uip_appdata, end - (u8_t *) uip_appdata);
}

//
//
//
static void send_request (void)
{
  u8_t *end;
  dhcpMsg_t *m = (dhcpMsg_t *) uip_appdata;

  create_msg (m);

  end = add_msg_type (&m->options [4], DHCPREQUEST);
  end = add_server_id (end);
  end = add_req_ipaddr (end);
  end = add_end (end);

  uip_send (uip_appdata, end - (u8_t *) uip_appdata);
}

//
//
//
static u8_t parse_options (u8_t *optptr, int len)
{
  u8_t *end = optptr + len;
  u8_t type = 0;

  while (optptr < end) 
  {
    switch (*optptr) 
    {
      case DHCP_OPTION_SUBNET_MASK :
        memcpy (dhcpcState->netmask, optptr + 2, 4);
        break;

      case DHCP_OPTION_TIME_OFFSET :
        memcpy (&dhcpcState->timeoffset, optptr + 2, 4);
        dhcpcState->timeoffset = ntohl (dhcpcState->timeoffset);
        break;

      case DHCP_OPTION_ROUTER :
        memcpy (dhcpcState->default_router, optptr + 2, 4);
        break;

      case DHCP_OPTION_DNS_SERVER :
        memcpy (dhcpcState->dnsaddr, optptr + 2, 4);
        break;

      case DHCP_OPTION_NTP_SERVER :
        memcpy (dhcpcState->sntpaddr, optptr + 2, 4);
        break;

      case DHCP_OPTION_MSG_TYPE :
        type = *(optptr + 2);
        break;

      case DHCP_OPTION_SERVER_ID :
        memcpy (dhcpcState->serverid, optptr + 2, 4);
        break;

      case DHCP_OPTION_LEASE_TIME :
        memcpy (dhcpcState->lease_time, optptr + 2, 4);
        break;

      case DHCP_OPTION_END :
        return type;
    }

    optptr += optptr [1] + 2;
  }

  return type;
}

//
//
//
static u8_t parse_msg (void)
{
  dhcpMsg_t *m = (dhcpMsg_t *) uip_appdata;

  if ((m->op == DHCP_REPLY) && !memcmp (m->xid, xid, sizeof (xid)) && !memcmp (m->chaddr, dhcpcState->mac_addr, dhcpcState->mac_len))
  {
    memcpy (dhcpcState->ipaddr, m->yiaddr, 4);
    return parse_options (&m->options [4], uip_datalen ());
  }

  return 0;
}

//
//
//
static PT_THREAD (handle_dhcp (void))
{
  PT_BEGIN (&dhcpcState->pt);

  dhcpcState->state = STATE_SENDING;
  dhcpcState->ticks = CLOCK_SECOND;

  do 
  {
    send_discover ();
    timer_set (&dhcpcState->timer, dhcpcState->ticks);

    PT_WAIT_UNTIL (&dhcpcState->pt, uip_newdata () || timer_expired (&dhcpcState->timer));

    if (uip_newdata () && (parse_msg () == DHCPOFFER)) 
    {
      uip_flags &= ~UIP_NEWDATA;
      dhcpcState->state = STATE_OFFER_RECEIVED;
      break;
    }

    uip_flags &= ~UIP_NEWDATA;

    if (dhcpcState->ticks < CLOCK_SECOND * 60)
      dhcpcState->ticks *= 2;
    else 
    {
      dhcpcState->ipaddr [0] = dhcpcState->ipaddr [1] = 0;
      goto dhcpcf;
    }
  } 
  while (dhcpcState->state != STATE_OFFER_RECEIVED);

  dhcpcState->ticks = CLOCK_SECOND;

  do 
  {
    send_request ();
    timer_set (&dhcpcState->timer, dhcpcState->ticks);

    PT_WAIT_UNTIL (&dhcpcState->pt, uip_newdata () || timer_expired (&dhcpcState->timer));

    if (uip_newdata () && (parse_msg () == DHCPACK))
    {
      uip_flags &= ~UIP_NEWDATA;
      dhcpcState->state = STATE_CONFIG_RECEIVED;
      break;
    }

    uip_flags &= ~UIP_NEWDATA;

    if (dhcpcState->ticks <= CLOCK_SECOND * 10)
      dhcpcState->ticks += CLOCK_SECOND;
    else
      PT_RESTART (&dhcpcState->pt);
  } 
  while (dhcpcState->state != STATE_CONFIG_RECEIVED);

dhcpcf:
  dhcpc_configured (dhcpcState);

  /*  timer_stop (&dhcpcState->timer);*/

  /*
   * PT_END restarts the thread so we do this instead. Eventually we
   * should reacquire expired leases here.
   */
  while (1)
    PT_YIELD (&dhcpcState->pt);

  PT_END (&dhcpcState->pt);
}

//
//
//
void dhcpc_init (const void *mac_addr, int mac_len)
{
  struct uip_udp_conn *conn;
  uip_ipaddr_t addr;

  uip_ipaddr (addr, 255,255,255,255);

  if ((conn = uip_udp_new (&addr, HTONS (DHCPC_SERVER_PORT))))
  {
    dhcpcState = (dhcpcState_t *) &conn->appstate;
    dhcpcState->conn = conn;
    dhcpcState->mac_addr = mac_addr;
    dhcpcState->mac_len  = mac_len;
    dhcpcState->state = STATE_INITIAL;

    uip_udp_bind (dhcpcState->conn, HTONS (DHCPC_CLIENT_PORT));
  }

  PT_INIT (&dhcpcState->pt);
}

//
//
//
void dhcpc_appcall (void)
{
  handle_dhcp ();
}

//
//
//
void dhcpc_request (void)
{
  u16_t ipaddr [2];

  if (dhcpcState->state == STATE_INITIAL) 
  {
    uip_ipaddr (ipaddr, 0,0,0,0);
    uip_sethostaddr (ipaddr);
  }
}
