//
//  $Id: a24c1024.c 302 2008-11-07 03:47:50Z jcw $
//  $Revision: 302 $
//  $Author: jcw $
//  $Date: 2008-11-06 22:47:50 -0500 (Thu, 06 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/a24c1024.c $
//

#include "FreeRTOS.h"

#include <string.h>

#include "i2c.h"
#include "a24c1024.h"

//
//
//
static U32 rwAddress;
static U8 deviceAddress = A24C1024_ADDRESS;

//
//
//
void a24c1024Init (void)
{
}

int a24c1024SetAddress (U32 address)
{
  int r = 0;

  if (address >= A24C1024_SIZE)
    r = -1;

  address %= A24C1024_SIZE;

  if ((rwAddress = address) >= 65536)
    deviceAddress |= 0x02;
  else
    deviceAddress &= ~0x02;

  return r;
}

static int a24c1024SetAddressEx (U32 address, U8 *buffer)
{
  int r;

  if ((r = a24c1024SetAddress (address)))
    return r;

  buffer [0] = rwAddress >> 8;
  buffer [1] = rwAddress;

  return 0;
}

U32 a24c1024GetAddress (void)
{
  return rwAddress;
}

//
//
//
int a24c1024Read (U8 *buffer, U32 bufferLength)
{
  int r;

  buffer [0] = rwAddress >> 8;
  buffer [1] = rwAddress;

  if (!(r = i2cWriteReadBufferPoll (deviceAddress, buffer, sizeof (U16), bufferLength)))
    a24c1024SetAddress (a24c1024GetAddress () + bufferLength);

  return r;
}

int a24c1024ReadAddress (U32 address, U8 *buffer, U32 bufferLength)
{
  int r;

  if ((r = a24c1024SetAddress (address)))
    return r;

  return a24c1024Read (buffer, bufferLength);
}

//
//
//
int a24c1024Write (U8 *buffer, U32 bufferLength)
{
  int r;

  buffer [0] = rwAddress >> 8;
  buffer [1] = rwAddress;

  if (!(r = i2cWriteBufferPoll (deviceAddress, buffer, bufferLength + 2)))
    a24c1024SetAddress (a24c1024GetAddress () + bufferLength);

  return r;
}

int a24c1024WriteAddress (U32 address, U8 *buffer, U32 bufferLength)
{
  int r;

  if ((r = a24c1024SetAddress (address)))
    return r;

  return a24c1024Write (buffer, bufferLength);
}

//
//
//
static int a24c1024FillAddressCommon (U32 address, U8 *buffer, U32 bufferLength)
{
  int r;

  if (!(r = a24c1024SetAddressEx (address, buffer)))
    r = i2cWriteBufferPoll (deviceAddress, buffer, bufferLength + 2);

  return r;
}

int a24c1024FillAddress (U32 address, U32 length, U8 fillValue)
{
  int r;
  U32 l;
  U32 i;
  U8 buffer [A24C1024_PAGESIZE + 2];

  memset (buffer, fillValue, sizeof (buffer));

  l = (A24C1024_PAGESIZE - ((address + A24C1024_PAGESIZE) % A24C1024_PAGESIZE)) % A24C1024_PAGESIZE;
  l = MIN (l, length);

  if (l && (r = a24c1024FillAddressCommon (address, buffer, l)))
    return r;

  address += l;
  length -= l;
  l = length - (length % A24C1024_PAGESIZE);

  for (i = 0; i < l; i += A24C1024_PAGESIZE, address += (sizeof (buffer) - 2), length -= (sizeof (buffer) - 2))
    if ((r = a24c1024FillAddressCommon (address, buffer, sizeof (buffer) - 2)))
      return r;

  if (length && (r = a24c1024FillAddressCommon (address, buffer, length)))
    return r;

  return 0;
}
