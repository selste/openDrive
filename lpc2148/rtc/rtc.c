//
//  $Id: rtc.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/rtc/rtc.c $
//

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>

#include "rtcISR.h"
#include "rtc.h"

//
//  Place RTC on 32kHz xtal and disconnect power.
//
static inline void rtcSleep (void)
{
  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
  SCB_PCONP &= ~SCB_PCONP_PCRTC;
}

//
//  Prepare clock for interactive use.
//
static inline void rtcWake (void)
{
  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
  SCB_PCONP |= SCB_PCONP_PCRTC;
}

//
//  Read clock registers and return tm structure.
//
static void rtcRead (struct tm *theTime, unsigned int *milliseconds)
{
  unsigned int ticks32Khz;
  rtcCTIME0_t ctime0;
  rtcCTIME1_t ctime1;
  rtcCTIME2_t ctime2;

  rtcWake ();
  portENTER_CRITICAL ();

  do
  {
    ctime0.i = RTC_CTIME0;
    ctime1.i = RTC_CTIME1;
    ctime2.i = RTC_CTIME2;

    do
      ticks32Khz = (RTC_CTC & 0xfffe);
    while (ticks32Khz != (RTC_CTC & 0xfffe));
  }
  while (ctime0.i != RTC_CTIME0);

  portEXIT_CRITICAL ();
  rtcSleep ();

  if (milliseconds)
    *milliseconds = (((ticks32Khz / 2) * 1000u) / 32768u);

  theTime->tm_sec   = ctime0.seconds;
  theTime->tm_min   = ctime0.minutes;
  theTime->tm_hour  = ctime0.hours;
  theTime->tm_mday  = ctime1.dom;
  theTime->tm_mon   = ctime1.month - 1;
  theTime->tm_year  = ctime1.year - 1900;
  theTime->tm_wday  = ctime0.dow;
  theTime->tm_yday  = ctime2.doy - 1;
  theTime->tm_isdst = 0;
}

//
//  Set clock to new values.
//
static void rtcWrite (struct tm *newTime)
{
  rtcWake ();
  portENTER_CRITICAL ();

  RTC_CCR &= ~RTC_CCR_CLKEN;
  RTC_CCR |=  RTC_CCR_CTCRST;

  RTC_SEC   = newTime->tm_sec;
  RTC_MIN   = newTime->tm_min;
  RTC_HOUR  = newTime->tm_hour;
  RTC_DOM   = newTime->tm_mday;
  RTC_MONTH = newTime->tm_mon + 1;
  RTC_YEAR  = newTime->tm_year + 1900;
  RTC_DOW   = newTime->tm_wday;
  RTC_DOY   = newTime->tm_yday + 1;

  RTC_CCR &= ~RTC_CCR_CTCRST;
  RTC_CCR |=  RTC_CCR_CLKEN;

  portEXIT_CRITICAL ();
  rtcSleep ();
}

//
//
//
time_t rtcGetEpochSeconds (unsigned int *milliseconds)
{
  struct tm tm;

  rtcRead (&tm, milliseconds);
  return mktime (&tm);
}

void rtcSetEpochSeconds (time_t now)
{
  struct tm tm;

  localtime_r (&now, &tm);
  rtcWrite (&tm);
}

//
//  Start clock so that the sytsem may use it.
//
static void rtcInitClockCalendar (void)
{
  int nonsense = 0;
  rtcCTIME0_t ctime0;
  rtcCTIME1_t ctime1;
  rtcCTIME2_t ctime2;

  rtcWake ();
  
  ctime0.i = RTC_CTIME0; 
  ctime1.i = RTC_CTIME1; 
  ctime2.i = RTC_CTIME2;

  //
  //  Leisurely tear the packed time apart into individual time.
  //
  if ((ctime0.seconds < 0) || (ctime0.seconds > 59))  nonsense = 1;
  if ((ctime0.minutes < 0) || (ctime0.minutes > 59))  nonsense = 1;
  if ((ctime0.hours < 0)   || (ctime0.hours > 23))    nonsense = 1;
  if ((ctime1.dom < 1)     || (ctime1.dom > 31))      nonsense = 1;
  if ((ctime1.month < 1)   || (ctime1.month > 12))    nonsense = 1;
  if ((ctime1.year < 1980) || (ctime1.year > 2050))   nonsense = 1;
  if ((ctime0.dow < 0)     || (ctime0.dow > 6))       nonsense = 1;
  if ((ctime2.doy < 0)     || (ctime2.doy > 366))     nonsense = 1;

  rtcSleep ();

  //
  //  Set the clock to Jan 1, 2006 00:00:00
  //
  if (nonsense) 
    rtcSetEpochSeconds ((time_t) 1136073600);
}

//
//
//
void rtcInit (void)
{
  rtcISRInit ();

  SCB_PCONP |= SCB_PCONP_PCRTC;

  RTC_CCR = 0;

  RTC_CCR |= RTC_CCR_CLKSRC;

  RTC_AMR = RTC_AMR_AMRMASK;
  RTC_CIIR = 0;

  RTC_ILR = RTC_ILR_MASK;

  RTC_CCR |= RTC_CCR_CLKEN;

  VIC_IntSelect &= ~VIC_IntSelect_RTC;
#ifdef RTC_NONVECTOREDIRQ
  VIC_DefVectAddr = (portLONG) rtcISR;
#else
  VIC_VectAddr6 = (portLONG) rtcISR;
  VIC_VectCntl6 = VIC_VectCntl_ENABLE | VIC_Channel_RTC;
#endif
  VIC_IntEnable = VIC_IntEnable_RTC;

  rtcInitClockCalendar ();
}

int rtcSetAlarm (struct tm *tm)
{
  if (tm && (mktime (tm) < time (NULL)))
    return -1;

  rtcWake ();

  RTC_AMR = RTC_AMR_AMRMASK;

  if (tm)
  {
    RTC_ALYEAR = tm->tm_year + 1900;
    RTC_ALMON = tm->tm_mon + 1;
    RTC_ALDOM = tm->tm_mday;
    RTC_ALHOUR = tm->tm_hour;
    RTC_ALMIN = tm->tm_min;
    RTC_ALSEC = tm->tm_sec;
    RTC_ALDOW = 0;
    RTC_ALDOY = 0;

    RTC_AMR = RTC_AMR_AMRDOW | RTC_AMR_AMRDOY;
  }

  rtcSleep ();

  return 0;
}

struct tm *rtcGetAlarmTm (struct tm *tm)
{
  if (tm)
  {
    memset (tm, 0, sizeof (* tm));

    rtcWake ();

    if (RTC_AMR != RTC_AMR_AMRMASK)
    {
      tm->tm_year = RTC_ALYEAR - 1900;
      tm->tm_mon = RTC_ALMON - 1;
      tm->tm_mday = RTC_ALDOM;
      tm->tm_hour = RTC_ALHOUR;
      tm->tm_min = RTC_ALMIN;
      tm->tm_sec = RTC_ALSEC;
    }
    else
    {
      time_t now = 0;
      memcpy (tm, localtime (&now), sizeof (* tm));
    }

    rtcSleep ();
  }

  return tm;
}

time_t rtcGetAlarmEpochSeconds (void)
{
  struct tm tm;

  return mktime (rtcGetAlarmTm (&tm));
}

int rtcPeriodicAlarm (int mode)
{
  int state;
    
  rtcWake ();

  state = RTC_CIIR & RTC_CIIR_IMMIN;

  if (!mode)
    RTC_CIIR &= ~RTC_CIIR_IMMIN;
  else if (mode > 0)
    RTC_CIIR |= RTC_CIIR_IMMIN;

  rtcSleep ();

  return state ? 1 : 0;
}
