//
//  $Id: i2c.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/i2c.c $
//

#include "FreeRTOS.h"

#include "i2c.h"

//
//
//
typedef struct i2cErrnoStr_s
{
  int errno;
  const char *text;
}
i2cErrnoStr_t;

static i2cErrnoStr_t i2cErrnoStr [] =
{
  { I2CERR_NONE,              "I2CERR_NONE" },
  { I2CERR_BUSY,              "I2CERR_BUSY" },
  { I2CERR_EMPTY,             "I2CERR_EMPTY" },
  { I2CERR_TIMEOUT,           "I2CERR_TIMEOUT" },
  { I2CERR_TIMEOUTWC,         "I2CERR_TIMEOUTWC" },
  { I2CERR_TIMEOUTACKPOLL,    "I2CERR_TIMEOUTACKPOLL" },
  { I2CERR_NOTIMPLEMENTED,    "I2CERR_NOTIMPLEMENTED" },
  { I2CERR_OTHER,             "I2CERR_OTHER" },
  { I2CERR_BUSERROR,          "I2CERR_BUSERROR" },
  { I2CERR_BUSERRORx,         "I2CERR_BUSERRORx" },
  { I2CERR_STARTTX,           "I2CERR_STARTTX" },
  { I2CERR_REPEATEDSTARTTX,   "I2CERR_REPEATEDSTARTTX" },
  { I2CERR_SLAWTX_ACKRX,      "I2CERR_SLAWTX_ACKRX" },
  { I2CERR_SLAWTX_NACKRX,     "I2CERR_SLAWTX_NACKRX" },
  { I2CERR_DATTX_ACKRX,       "I2CERR_DATTX_ACKRX" },
  { I2CERR_DATTX_NACKRX,      "I2CERR_DATTX_NACKRX" },
  { I2CERR_ARBLOST,           "I2CERR_ARBLOST" },
  { I2CERR_SLARTX_ACKRX,      "I2CERR_SLARTX_ACKRX" },
  { I2CERR_SLARTX_NACKRX,     "I2CERR_SLARTX_NACKRX" },
  { I2CERR_DATRX_ACKTX,       "I2CERR_DATRX_ACKTX" },
  { I2CERR_DATRX_NACKTX,      "I2CERR_DATRX_NACKTX" },
  { I2CERR_NOINFO,            "I2CERR_NOINFO" },
};

i2cErr_e i2cErrno = I2CERR_NONE;

//
//
//
int i2cGetErrno (void)
{
  return i2cErrno;
}

const char *i2cStrerror (int err)
{
  unsigned int i;

  for (i = 0; i < arrsizeof (i2cErrnoStr); i++)
    if (i2cErrnoStr [i].errno == err)
      return i2cErrnoStr [i].text;

  return NULL;
}
