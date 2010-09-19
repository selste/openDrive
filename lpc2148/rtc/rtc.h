//
//  $Id: rtc.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/rtc/rtc.h $
//

#ifndef _RTC_H_
#define _RTC_H_

#include <time.h>

//
//  Define this for using non-vectored IRQs.  Undef it if a regular vectored
//  IRQ is preferred.
//
#define RTC_NONVECTOREDIRQ

//
//
//
void rtcInit (void);
time_t rtcGetEpochSeconds (unsigned int *milliseconds);
void rtcSetEpochSeconds (time_t now);
int rtcSetAlarm (struct tm *tm);
struct tm *rtcGetAlarmTm (struct tm *tm);
time_t rtcGetAlarmEpochSeconds (void);
int rtcPeriodicAlarm (int mode);

#endif
