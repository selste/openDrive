//
//  $Id: sntp.c 331 2008-11-09 16:59:47Z jcw $
//  $Revision: 331 $
//  $Author: jcw $
//  $Date: 2008-11-09 11:59:47 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/apps/sntp/sntp.c $
//

#include <string.h>
#include <time.h>

#include "../../uip/uip.h"
#include "../../uip/uip_timer.h"
#include "../../uip/pt.h"
#include "sntp.h"

//
//
//
#define JAN_1970 0x83aa7e80 /* 2208988800 1970 - 1900 in seconds */

#define NTP_TO_UNIX(n,u) do {  u = n - JAN_1970; } while (0)

#define NTOHL_FP(n, h)  do { (h)->l_ui = ntohl((n)->l_ui); \
                             (h)->l_uf = ntohs((n)->l_uf); } while (0)

//
//
//
typedef enum
{
  LEAPINDICATOR_NO_WARNING = 0,
  LEAPINDICATOR_61_SECOND_DAY,
  LEAPINDICATOR_59_SECOND_DAY,
  LEAPINDICATOR_ALARM,
}
leapIndicator_e;

typedef enum
{
  MODE_RESERVED = 0,
  MODE_SYMMETRIC_ACTIVE,
  MODE_SYMMETRIC_PASSIVE,
  MODE_CLIENT,
  MODE_SERVER,
  MODE_BROADCAST,
  MODE_NTP_CONTROL_MSG,
  MODE_PRIVATE,
}
mode_e;

//
//  RFC1305
//
#define l_ui Ul_i.Xl_ui /* unsigned integral part */
#define l_i  Ul_i.Xl_i  /* signed integral part */
#define l_uf Ul_f.Xl_uf /* unsigned fractional part */
#define l_f  Ul_f.Xl_f  /* signed fractional part */

typedef struct 
{
  union 
  {
    u32_t Xl_ui;
    n32_t Xl_i;
  } 
  Ul_i;

  union 
  {
    u32_t Xl_uf;
    n32_t Xl_f;
  } 
  Ul_f;
} 
__attribute__ ((packed)) l_fp;

typedef struct sntpHeader_s
{
  u16_t mode          : 3;
  u16_t versionNumber : 3;
  u16_t leapIndicator : 2;
  u16_t stratum       : 8;
  u16_t poll          : 8;
  u16_t precision     : 8;
  u32_t rootDelay;
  u32_t rootDispersion;
  u32_t refID;
  l_fp  refTimeStamp;
  l_fp  orgTimeStamp;
  l_fp  rxTimeStamp;
  l_fp  txTimeStamp;
}
__attribute__ ((packed)) sntpHeader_t;

//
//
//
static sntpState_t *sntpState;

//
//
//
int sntpSync (uip_ipaddr_t *ipaddr, void (*syncedCallback) (time_t *epochSeconds))
{
	struct uip_udp_conn *conn;

  if (sntpState && sntpState->conn)
    uip_udp_remove (sntpState->conn);

  if (!(conn = uip_udp_new (ipaddr, HTONS (123))))
    return 0;

  sntpState = (sntpState_t *) &conn->appstate;

  sntpState->conn = conn;
  sntpState->onSyncCallback = syncedCallback;
  sntpState->retry = SNTP_RETRIES;

  PT_INIT (&sntpState->pt);

  return 1;
}

//
//
//
static void sntpUpdate (void)
{
  sntpHeader_t *hdr = (sntpHeader_t *) uip_appdata;

  memset (hdr, 0, sizeof (sntpHeader_t));

  hdr->leapIndicator = LEAPINDICATOR_ALARM;
  hdr->versionNumber = 4;
  hdr->mode = MODE_CLIENT;

  uip_udp_send (sizeof (sntpHeader_t));
}

//
//
//
static void sntpConvertToTm (sntpHeader_t *h, time_t *epochSeconds)
{
  sntpHeader_t temp;

  NTOHL_FP (&h->txTimeStamp, &temp.txTimeStamp);
  NTP_TO_UNIX (temp.txTimeStamp.Ul_i.Xl_ui, *epochSeconds);
}

//
//
//
static PT_THREAD (sntpHandler (void))
{
  PT_BEGIN (&sntpState->pt);

  do
  {
    sntpUpdate ();

    timer_set (&sntpState->timer, (SNTP_TIMEOUT * CLOCK_SECOND));

    PT_WAIT_UNTIL (&sntpState->pt, (!uip_poll () && uip_newdata ()) || timer_expired (&sntpState->timer));

    sntpState->retry--;
  }
  while (uip_poll () && sntpState->retry);

  if (timer_expired (&sntpState->timer))
  {
    if (*sntpState->onSyncCallback)
      (*sntpState->onSyncCallback) (NULL);

    uip_udp_remove (sntpState->conn);
  }
  else
  {
    if (((sntpHeader_t *) uip_appdata)->refID)
    {
      time_t epochSeconds;

      sntpConvertToTm ((sntpHeader_t *) uip_appdata, &epochSeconds);

      if (*sntpState->onSyncCallback)
        (*sntpState->onSyncCallback) (&epochSeconds);

      uip_udp_remove (sntpState->conn);
    }
    else if (*sntpState->onSyncCallback)
      (*sntpState->onSyncCallback) (NULL);
  }

  while (1)
    PT_YIELD (&sntpState->pt);

  PT_END (&sntpState->pt);
}

//
//
//
void sntp_appcall (void)
{
  if (sntpState)
    sntpHandler ();
}
