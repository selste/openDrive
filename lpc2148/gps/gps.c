//
//  $Id: gps.c 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/gps/gps.c $
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "../main.h"
#include "gps.h"

//
//
//
#define GPS_MAX_NMEA_SENTENCE 128

//
//  If CFG_GPS is not defined, we still compile the code, it's just not included.
//  Define TASKHANDLE_GPS so we compile.  Normally this is defined in main.h,
//  but it's #ifndef'ed out so that we don't allocate a task handle slot for it
//  when GPS support isn't included.
//
#ifndef CFG_GPS
#define TASKHANDLE_GPS 0
#endif

//
//
//
typedef enum
{
  SCANTYPE_SKIP = 0,
  SCANTYPE_INT,
  SCANTYPE_FLOAT,
  SCANTYPE_DOUBLE,
  SCANTYPE_CHAR,
  SCANTYPE_STRING,
  SCANTYPE_END,
}
scanType_e;

typedef struct scan_s
{
  scanType_e scanType;
  void *value;
}
scan_t;

//
//
//
static gpsData_t gpsData;
static xSemaphoreHandle semaphore = NULL;

//
//
//
static void gpsNormalizeNMEA (char *s);
static unsigned char gpsChecksumNMEA (char *sz);
static void gpsHandlerGGA (char *nmeaSentence);
static void gpsHandlerRMC (char *nmeaSentence);
static void gpsHandlerRestart (char *nmeaSentence);
static void gpsDispatchMessages (char *nmeaSentence);
static int gpsProcessByte (unsigned char c, char *nmeaSentence);

//
//
//
static int hatoi (const char *s)
{
  const unsigned char hexToDec [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15};
  int value = 0;

  while (*s && isxdigit (*s))
    value = (value << 4) | hexToDec [toupper (*s++) - '0'];

  return value;
}

//
//
//
static void gpsNormalizeNMEA (char *s)
{
  int l = -1;

  while (*s && *s != '*')
  {
    if (*s == ',' && (*(s + 1) == ',' || *(s + 1) == '*'))
    {
      if (l == -1)
        l = strlen (s) + 1;

      memmove (s + 3, s + 1, l);
      *++s = '-';
      *++s = '1';
      l += 2;
    }

    s++;

    if (l != -1)
      l--;
  }
}

//
//
//
static unsigned char gpsChecksumNMEA (char *sz)
{
  short i;
  unsigned char cs;

  for (cs = 0, i = 1; sz [i] && sz [i] != '*'; i++)
    cs ^= ((unsigned char) sz [i]);

  return cs;
}

//
//
//
static char *gpsScanNextField (char *sentence)
{
  if (*sentence != ',')
    while (*sentence && (*sentence != ','))
      sentence++;

  if (*sentence)
    sentence++;

  return sentence;
}

static int gpsScan (scan_t *scanList, char *sentence)
{
  int processed = 0;

  while (scanList)
  {
    if ((scanList->scanType == SCANTYPE_END) || !*sentence)
      return processed;

    if ((scanList->scanType != SCANTYPE_SKIP) && (*sentence == ','))
      return processed;

    if (scanList->scanType == SCANTYPE_INT)
      *(int *) scanList->value = atoi (sentence);
    else if (scanList->scanType == SCANTYPE_FLOAT)
      *(float *) scanList->value = atof (sentence);
    else if (scanList->scanType == SCANTYPE_DOUBLE)
      *(double *) scanList->value = atof (sentence);
    else if (scanList->scanType == SCANTYPE_CHAR)
      *(char *) scanList->value = *sentence;
    else
      processed--;

    processed++;
    sentence = gpsScanNextField (sentence);
    scanList++;
  }

  return processed;
}

//
//
//
static void gpsHandlerGGA (char *nmeaSentence)
{
  char   valid = 0;
  double height = 0.0;
  double latitude = 0.0, longitude = 0.0;
  char   latitudeSign = 0, longitudeSign = 0;

  scan_t scanList [] = 
  {
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_DOUBLE, &latitude },
    { SCANTYPE_CHAR,   &latitudeSign },
    { SCANTYPE_DOUBLE, &longitude },
    { SCANTYPE_CHAR,   &longitudeSign },
    { SCANTYPE_CHAR,   &valid },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_DOUBLE, &height },
    { SCANTYPE_END,    NULL },
  };

  if (gpsScan (scanList, nmeaSentence) == 6)
  {
    if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
    {
      gpsData.valid = (valid - '0');
      gpsData.height = height;
      gpsData.latitude  = latitude * (latitudeSign == 'N' ? 1.0 : -1.0);
      gpsData.longitude = longitude * (longitudeSign == 'E' ? 1.0 : -1.0);

      xSemaphoreGive (semaphore);
    }
  }
}

static void gpsHandlerRMC (char *nmeaSentence)
{
  float speed, course;
  int gpsdate, gpstime;

  scan_t scanList [] = 
  {
    { SCANTYPE_INT,    &gpstime },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_SKIP,   NULL },
    { SCANTYPE_FLOAT,  &speed },
    { SCANTYPE_FLOAT,  &course },
    { SCANTYPE_INT,    &gpsdate },
    { SCANTYPE_END,    NULL },
  };

  if (gpsScan (scanList, nmeaSentence) == 4)
  {
    if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
    {
      gpsData.utcDay      =  gpsdate / 10000;
      gpsData.utcMonth    = (gpsdate / 100) % 100;
      gpsData.utcYear     = (gpsdate % 100) + 2000;
      gpsData.utcHours    =  gpstime / 10000;
      gpsData.utcMinutes  = (gpstime / 100) % 100;
      gpsData.utcSeconds  =  gpstime % 100;
      gpsData.groundSpeed =  speed;
      gpsData.trueCourse  =  course;

      xSemaphoreGive (semaphore);
    }
  }
}

static void gpsHandlerRestart (char *nmeaSentencemea __attribute__ ((unused)))
{
  if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
  {
    gpsData.restarts++;
    xSemaphoreGive (semaphore);
  }
}

//
//
//
typedef struct nmeaDispatch_s
{
  const char *sentence;
  short length;
  void (*handler) (char *sentence);
  short normalize;
}
nmeaDispatch_t;

static const nmeaDispatch_t nmeaDispatch [] =
{
  { "$GPGGA",    6, gpsHandlerGGA,      1 },
  { "$GPRMC",    6, gpsHandlerRMC,      1 },
  { "$HW Type",  8, gpsHandlerRestart,  0 },
  { NULL,        0, NULL,               0 }
};

static void gpsDispatchMessages (char *nmeaSentence)
{
  int i;

  for (i = 0; nmeaDispatch [i].handler; i++)
  {
    if (!strncmp (nmeaDispatch [i].sentence, nmeaSentence, nmeaDispatch [i].length))
    {
      if (nmeaDispatch [i].normalize)
      {
        gpsNormalizeNMEA (&nmeaSentence [7]);
        (*nmeaDispatch [i].handler) (&nmeaSentence [7]);
      }
      else
        (*nmeaDispatch [i].handler) (NULL);

      break;
    }
  }
}

//
//
//
static int gpsProcessByte (unsigned char c, char *nmeaSentence)
{
  short complete = 0;
  static short state = 0;
  static short pos = 0;

  switch (state)
  {
    case 0 :
      {
        if (c == '$')
        {
          pos = 0;
          nmeaSentence [pos++] = '$';
          nmeaSentence [pos] = '\0';
          state = 1;
        }
        else
          state = 0;
      }
      break;

    case 1 :
      {
        if (pos < GPS_MAX_NMEA_SENTENCE)
        {
          if (c == 0x0a)
          {
            char *s;

            state = 0;

            if ((s = strchr (nmeaSentence, '*')))
            {
              int cksum;

              if (gpsChecksumNMEA (nmeaSentence) == (cksum = hatoi (s + 1)))
                complete = 1;
              else
                printf ("NMEA checksum error: got 0x%02x, want %s", cksum, s);
            }
            else
              printf ("NMEA checksum not found: \"%s\"", nmeaSentence);
          }
          else if (c != 0x0d)
          {
            nmeaSentence [pos++] = c;
            nmeaSentence [pos] = '\0';
          }
        }
        else
          state = 0;
      }
      break;
  }

  return (complete);
}

//
//  Return 1 if got a copy, 0 if not.
//
int gpsCopyData (gpsData_t *dst)
{
  if (semaphore && xSemaphoreTake (semaphore, 100 / portTICK_RATE_MS) == pdTRUE)
  {
    memcpy (dst, &gpsData, sizeof (gpsData_t));
    xSemaphoreGive (semaphore);
    return 1;
  }

  memset (dst, 0, sizeof (gpsData_t));
  return 0;
}

//
//
//
static portTASK_FUNCTION (vGPSTask, pvParameters __attribute__ ((unused)))
{
  int fd;
  static char nmeaSentence [GPS_MAX_NMEA_SENTENCE];

  memset (&gpsData, 0, sizeof (gpsData));

  if (!semaphore)
    vSemaphoreCreateBinary (semaphore);

  fd = open ("/dev/uart1", O_RDONLY);

  if ((fd == -1) || (semaphore == NULL))
    for (;;)
      vTaskDelay (100);

  for (;;)
  {
    portCHAR c;

    if (read (fd, &c, sizeof (c)) == sizeof (c))
      if (gpsProcessByte (c, nmeaSentence))
        gpsDispatchMessages (nmeaSentence);
  }
}

//
//
//
signed portBASE_TYPE gpsTaskStart (void)
{
  return xTaskCreate (vGPSTask, (const signed portCHAR * const) "GPS", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &taskHandles [TASKHANDLE_GPS]);
}

signed portBASE_TYPE gpsTaskStop (void)
{
  if (!taskHandles [TASKHANDLE_GPS])
    return 0;
  
  vTaskDelete (taskHandles [TASKHANDLE_GPS]);
  taskHandles [TASKHANDLE_GPS] = NULL;
  return 1;
}
