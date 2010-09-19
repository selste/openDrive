//
//  $Id: lm75.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/lm75.c $
//

#include "FreeRTOS.h"

#include "i2c.h"
#include "lm75.h"

//
//
//
static U8 lm75Address = LM75_ADDRESS;
static U8 lm75LastRegister = LM75_REGISTER_TEMPERATURE;
static U8 lm75Mode = 0;

//
//
//
static int lm75Read8 (U8 reg, int *value)
{
  lm75LastRegister = reg;
  *value = 0;

  if (i2cWriteBuffer (lm75Address, (U8 *) &reg, sizeof (U8)))
    return -1;

  return i2cReadBuffer (lm75Address, (U8 *) value, sizeof (U8));
}

static int lm75Write8 (U8 reg, int value)
{
  U8 buffer [2];

  buffer [0] = lm75LastRegister = reg;
  buffer [1] = value;

  return i2cWriteBuffer (lm75Address, buffer, sizeof (buffer));
}

//
//  DANGER, WILL ROBINSON!  'value' is the desired temperature * 2, where
//  the low bit indicates 0.5C or not.  So for -50C degrees, we see -100.
//  For -50.5C, we see -101.
//
static int lm75Read16 (U8 reg, int *value)
{
  U8 buffer [2];

  buffer [0] = lm75LastRegister = reg;

  if (!lm75Mode)
  {
    if (i2cWriteReadBuffer (lm75Address, buffer, sizeof (U8), sizeof (buffer)))
      return -1;
  }
  else
  {
    if (i2cWriteBuffer (lm75Address, (U8 *) &reg, sizeof (U8)))
      return -1;

    if (i2cReadBuffer (lm75Address, buffer, sizeof (buffer)))
      return -1;
  }

  //
  //  Sign extend negative numbers
  //
  *value = ((buffer [0] << 8) | buffer [1]) >> 7;

  if (buffer [0] & 0x80)
    *value |= 0xfffffe00;

  return I2CERR_NONE;
}

static int lm75Write16 (U8 reg, int value)
{
  U8 buffer [3];

  value <<= 7;

  buffer [0] = lm75LastRegister = reg;
  buffer [1] = value >> 8;
  buffer [2] = value;

  return i2cWriteBuffer (lm75Address, buffer, sizeof (buffer));
}

//
//  Don't call lm75Init() before tasker is started.  The I2C routines use the
//  newlib/syscalls.c _times function, which calls xTaskGetTickCount(), which
//  hasn't been initialized yet.
//
int lm75Init (void)
{
  return lm75ConfigWrite (0);
}

void lm75SetMode (int mode)
{
  lm75Mode = mode;
}

void lm75SetAddress (U8 address)
{
  lm75Address = address;
}

int lm75ReRead (int *value)
{
  *value = 0;

  if (lm75LastRegister == LM75_REGISTER_CONFIGURATION)
    return i2cReadBuffer (lm75Address, (U8 *) value, sizeof (U8));
  else
  {
    U8 buffer [2];

    if (i2cReadBuffer (lm75Address, buffer, sizeof (buffer)))
      return -1;

    *value = ((buffer [0] << 8) | buffer [1]) >> 7;
  }

  return I2CERR_NONE;
}

int lm75TemperatureRead (int *temp)
{
  return lm75Read16 (LM75_REGISTER_TEMPERATURE, temp);
}

int lm75ConfigRead (int *configValue)
{
  return lm75Read8 (LM75_REGISTER_CONFIGURATION, configValue);
}

int lm75ConfigWrite (int configValue)
{
  return lm75Write8 (LM75_REGISTER_CONFIGURATION, configValue);
}

int lm75THYSTRead (int *thystValue)
{
  return lm75Read16 (LM75_REGISTER_THYST, thystValue);
}

int lm75THYSTWrite (int thystValue)
{
  return lm75Write16 (LM75_REGISTER_THYST, thystValue);
}

int lm75TOSTRead (int *thystValue)
{
  return lm75Read16 (LM75_REGISTER_TOS, thystValue);
}

int lm75TOSWrite (int thystValue)
{
  return lm75Write16 (LM75_REGISTER_TOS, thystValue);
}
