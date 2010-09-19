//
//  $Id: i2c.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/i2c.h $
//

#ifndef _I2C_H_
#define _I2C_H_

#include "sysdefs.h"

//
//  Does not define slave RX statuses.  Bus errors (I2C_STAT == 0) is remapped
//  to I2CERR_ERROR in i2cStatus() so that we can use a value of 0 to indicate 
//  no error.
//
typedef enum
{
  I2CERR_NONE = 0,
  I2CERR_BUSY,
  I2CERR_EMPTY,
  I2CERR_TIMEOUT,
  I2CERR_TIMEOUTWC,
  I2CERR_TIMEOUTACKPOLL,
  I2CERR_NOTIMPLEMENTED,
  I2CERR_OTHER,
  I2CERR_BUSERROR,
  I2CERR_BUSERRORx       = 0x00,
  I2CERR_STARTTX         = 0x08,
  I2CERR_REPEATEDSTARTTX = 0x10,
  I2CERR_SLAWTX_ACKRX    = 0x18,
  I2CERR_SLAWTX_NACKRX   = 0x20,
  I2CERR_DATTX_ACKRX     = 0x28,
  I2CERR_DATTX_NACKRX    = 0x30,
  I2CERR_ARBLOST         = 0x38,
  I2CERR_SLARTX_ACKRX    = 0x40,
  I2CERR_SLARTX_NACKRX   = 0x48,
  I2CERR_DATRX_ACKTX     = 0x50,
  I2CERR_DATRX_NACKTX    = 0x58,
  I2CERR_NOINFO          = 0xf8
}
i2cErr_e;

//
//
//
extern i2cErr_e i2cErrno;

//
//
//
void i2cInit (void);
int i2cGetErrno (void);
const char *i2cStrerror (int err);
void i2cSetTimeout (unsigned int timeoutInMilliseconds);
void i2cDump (void);
int i2cWriteBuffer (U8 address, U8 *buffer, U32 bufferLength);
int i2cReadBuffer (U8 address, U8 *buffer, U32 bufferLength);
int i2cWriteReadBuffer (U8 address, U8 *buffer, U32 putLength, U32 getLength);
int i2cWriteBufferPoll (U8 address, U8 *buffer, U32 bufferLength);
int i2cWriteReadBufferPoll (U8 address, U8 *buffer, U32 putLength, U32 getLength);


#endif
