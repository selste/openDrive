//
//  $Id: i2cInt.c 143 2008-10-13 16:02:41Z jcw $
//  $Revision: 143 $
//  $Author: jcw $
//  $Date: 2008-10-13 12:02:41 -0400 (Mon, 13 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/i2cInt.c $
//

#include "FreeRTOS.h"

#include <stdio.h>
#include <string.h>
#include <sys/times.h>

#include "i2c.h"

//
//  Default timeout, in milliseconds for generic read/write
//
#define I2C_DEFAULT_TIMEOUT 100
#undef  I2C_DEBUG

//
//
//
typedef enum
{
  ACKPOLL_NO = 0,
  ACKPOLL_YES
}
ackPoll_e;

//
//
//
static volatile int i2cBusInUse;
static U8 i2cAddress;
static U8 *i2cDataBuffer;
static int i2cDataLenWrite;
static int i2cDataLenRead;
static unsigned int i2cTimeoutInTicks = I2C_DEFAULT_TIMEOUT / portTICK_RATE_MS;
static int i2cDataCnt;
static ackPoll_e i2cAckPoll = ACKPOLL_NO;

//
//
//
#ifdef I2C_DEBUG
static U8 i2cTransactions [64];
static int i2cTransactionsIndex;
#define i2cDebug(x) do { if (i2cAckPoll && (i2cTransactionsIndex < (int) sizeof (i2cTransactions))) i2cTransactions [i2cTransactionsIndex++] = x; } while (0)
#define i2cDebugReset() do { i2cTransactionsIndex = 0; } while (0)
#else
#define i2cDebug(x) do { } while (0)
#define i2cDebugReset() do { } while (0)
#endif

static void i2cISR_Handler (void)
{
  i2cErrno = (I2C0_STAT & I2C_STAT_STATMASK);
  i2cDebug (i2cErrno);

  switch (i2cErrno)
  {
    //
    //  Transmit conditions
    //
    case I2CERR_BUSERRORx : // 0x00
      {
        I2C0_CONSET = I2C_CONSET_STO | I2C_CONSET_AA;
        i2cAckPoll = ACKPOLL_NO;
        i2cBusInUse = FALSE;
      }
      break;

    case I2CERR_STARTTX : // 0x08
    case I2CERR_REPEATEDSTARTTX : // 0x10
      {
        i2cDebug (i2cAddress);
        I2C0_DAT = i2cAddress;
      }
      break;

    case I2CERR_SLAWTX_ACKRX : // 0x18
    case I2CERR_DATTX_ACKRX : // 0x28
      {
        i2cAckPoll = ACKPOLL_NO;

        if (i2cDataLenWrite && (i2cDataCnt < i2cDataLenWrite))
        {
          i2cDebug (i2cDataBuffer [i2cDataCnt]);
          I2C0_DAT = i2cDataBuffer [i2cDataCnt++];
          I2C0_CONCLR = I2C_CONCLR_STAC;
        }
        else
        {
          if (!i2cDataLenRead)
          {
            i2cDebug (0xff);
            I2C0_CONCLR = I2C_CONCLR_STAC;
            I2C0_CONSET = I2C_CONSET_STO;
            i2cErrno = I2CERR_NONE;
            i2cBusInUse = FALSE;
          }
          else
          {
            i2cDebug (0xfe);
            i2cAddress |= 0x01;
            i2cDataCnt = 0;
            I2C0_CONSET = I2C_CONSET_STA;
          }
        }
      }
      break;

    case I2CERR_DATRX_NACKTX : // 0x58
      {
        i2cDataBuffer [i2cDataCnt++] = I2C0_DAT;
        i2cDebug (i2cDataBuffer [i2cDataCnt - 1]);
        i2cErrno = I2CERR_NONE;
      }

    case I2CERR_SLAWTX_NACKRX : // 0x20
    case I2CERR_SLARTX_NACKRX : // 0x48
      {
        if (i2cAckPoll)
          break;
      }

    case I2CERR_DATTX_NACKRX : // 0x30
      {
        I2C0_CONCLR = I2C_CONCLR_STAC;
        I2C0_CONSET = I2C_CONSET_STO;
        i2cBusInUse = FALSE;
      }
      break;

    case I2CERR_ARBLOST : // 0x38
      {
        I2C0_CONSET = I2C_CONSET_STA;
      }
      break;

    //
    //  Receive byte conditions (fall through is intentional)
    //
    case I2CERR_DATRX_ACKTX : // 0x50
      {
        i2cDataBuffer [i2cDataCnt++] = I2C0_DAT;
        i2cDebug (i2cDataBuffer [i2cDataCnt - 1]);
      }

    case I2CERR_SLARTX_ACKRX : // 0x40
      {
        if (i2cDataCnt < i2cDataLenRead - 1)
        {
          I2C0_CONCLR = I2C_CONCLR_STAC;
          I2C0_CONSET = I2C_CONSET_AA;
        }
        else
          I2C0_CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_AAC;
      }
      break;

    case I2CERR_NOINFO :
      break;

    default:
      {
        I2C0_CONCLR = I2C_CONCLR_I2ENC;
        i2cAckPoll = ACKPOLL_NO;
        i2cBusInUse = FALSE;
      }
      break;
  }

  I2C0_CONCLR = I2C_CONSET_SI;
  VIC_VectAddr = 0;
}

static void i2cISR (void) __attribute__ ((naked));
static void i2cISR (void)
{
  portSAVE_CONTEXT ();
  i2cISR_Handler ();
  portRESTORE_CONTEXT ();
}

//
// i2c1Init
//
void i2cInit (void)
{
  i2cBusInUse = FALSE;
  i2cAckPoll = ACKPOLL_NO;

  PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P02_MASK | PCB_PINSEL0_P03_MASK)) | (PCB_PINSEL0_P02_SCL0 | PCB_PINSEL0_P03_SDA0);

  SCB_PCONP |= SCB_PCONP_PCI2C0;

  I2C0_CONCLR = I2C_CONCLR_MASK;
  I2C0_CONSET = I2C_CONSET_I2EN;
  I2C0_SCLL   = 240;
  I2C0_SCLH   = 240;

  //
  //  Initialize the interrupt vector
  //
  VIC_IntSelect &= ~VIC_IntSelect_I2C0;
  VIC_VectCntl7 = VIC_VectCntl_ENABLE | VIC_Channel_I2C0;
  VIC_VectAddr7 = (int) i2cISR;
  VIC_IntEnable = VIC_IntEnable_I2C0;
}

//
//  Waits for the interrupt handler to mark the I2C bus as no longer in use, then
//  return 0 on success or -1 if any error occurred.
//
static int i2cWaitComplete (int ticks)
{
  if (i2cBusInUse)
  {
    clock_t theFuture;

    for (theFuture = times (NULL) + ticks; i2cBusInUse; )
    {
      if (times (NULL) > theFuture)
      {
        I2C0_CONCLR = I2C_CONCLR_I2ENC;
        i2cErrno = I2CERR_TIMEOUTWC;
        return -1;
      }
    }
  }

  return (i2cErrno == I2CERR_NONE) ? 0 : -1;
}

//
//  Disables interrupts then starts sending repeated STARTs until the slave device
//  returns ACK.  This indicates the slave has completed any operation in progress.
//
static int i2cPoll (U8 address)
{
  clock_t theFuture = times (NULL) + 10;

  VIC_IntEnClr = VIC_IntEnClr_I2C0;

  while (times (NULL) < theFuture)
  {
    I2C0_CONCLR = I2C_CONSET_SI;
    I2C0_CONSET = I2C_CONSET_STA;

    while (!(I2C0_CONSET & I2C_CONSET_SI))
      ;

    I2C0_CONCLR = I2C_CONCLR_STAC | I2C_CONCLR_SIC;
    I2C0_DAT = address & ~0x01;

    while (!(I2C0_CONSET & I2C_CONSET_SI))
      ;

    if ((i2cErrno = I2C0_STAT) == I2CERR_SLAWTX_ACKRX)
      break;
  }

  if (i2cErrno != I2CERR_SLAWTX_ACKRX)
    i2cErrno = I2CERR_TIMEOUTACKPOLL;

  I2C0_CONCLR = I2C_CONCLR_SIC;
  I2C0_CONSET = I2C_CONSET_STO | I2C_CONSET_AA;

  while (I2C0_CONSET & I2C_CONSET_STO)
    ;

  VIC_IntEnable = VIC_IntEnable_I2C0;

  return (i2cErrno == I2CERR_SLAWTX_ACKRX) ? 0 : -1;
}

//
//
//
static int i2cKickOff (U8 address, U8 *buffer, int bufferLenWrite, int bufferLenRead, ackPoll_e ackPoll)
{
  //
  //  Determine if our first operation will be a write or read.  If both, the
  //  write always occurs first.
  //
  if (bufferLenWrite)
    address &= ~0x01;
  else if (bufferLenRead)
    address |= 0x01;
  else
    address &= ~0x01;

  //
  //  Wait until last I2C operation has finished.  
  //
  if (i2cBusInUse && i2cWaitComplete (i2cTimeoutInTicks))
  {
    i2cErrno = I2CERR_TIMEOUT;
    return -1;
  }

  //
  //  Mark bus as in use, save the address, buffer and length
  //
  i2cBusInUse = TRUE;
  i2cAddress = address;
  i2cDataBuffer = buffer;
  i2cDataLenWrite = bufferLenWrite;
  i2cDataLenRead = bufferLenRead;
  i2cDataCnt = 0;
  i2cAckPoll = ackPoll;
  i2cDebugReset ();

  //
  //  Clear any outstanding bits, enable the interface, and send a start bit.
  //  Once the start bit goes out, the interrupt drive state machine begins.
  //
  I2C0_CONCLR = I2C_CONCLR_MASK;
  I2C0_CONSET = I2C_CONSET_I2EN;
  I2C0_CONSET = I2C_CONSET_STA;

  return 0;
}

//
//
//
static int i2cWriteBufferEx (U8 address, U8 *buffer, U32 bufferLength, int milliseconds)
{
  int r;

  if (!(r = i2cKickOff (address, buffer, bufferLength, 0, ACKPOLL_NO)))
    r = i2cWaitComplete (milliseconds / portTICK_RATE_MS);

  return r;
}

static int i2cReadBufferEx (U8 address, U8 *buffer, U32 bufferLength, int milliseconds)
{
  int r;

  if (!(r = i2cKickOff (address, buffer, 0, bufferLength, ACKPOLL_NO)))
    r = i2cWaitComplete (milliseconds / portTICK_RATE_MS);

  return r;
}

static int i2cWriteReadBufferEx (U8 address, U8 *buffer, U32 putLength, U32 getLength, int milliseconds)
{
  int r;

  if (!(r = i2cKickOff (address, buffer, putLength, getLength, ACKPOLL_NO)))
    r = i2cWaitComplete (milliseconds / portTICK_RATE_MS);

  return r;
}

//
//
//
void i2cSetTimeout (unsigned int timeoutInMilliseconds)
{
  if (timeoutInMilliseconds < portTICK_RATE_MS)
    timeoutInMilliseconds = portTICK_RATE_MS;

  i2cTimeoutInTicks = timeoutInMilliseconds / portTICK_RATE_MS;
}

void i2cDump (void)
{
#ifdef I2C_DEBUG
  int i;

  for (i = 0; i < i2cTransactionsIndex; i++)
    printf ("0x%02x ", i2cTransactions [i]);

  printf ("\n");
#else
  printf ("Not compiled for i2c debugging\n");
#endif
}

//
//  DANGER, WILL ROBINSON!  The callers buffer must persist until we're done
//
int i2cWriteBuffer (U8 address, U8 *buffer, U32 bufferLength)
{
  return i2cWriteBufferEx (address, buffer, bufferLength, i2cTimeoutInTicks);
}

int i2cReadBuffer (U8 address, U8 *buffer, U32 bufferLength)
{
  return i2cReadBufferEx (address, buffer, bufferLength, i2cTimeoutInTicks);
}

int i2cWriteReadBuffer (U8 address, U8 *buffer, U32 putLength, U32 getLength)
{
  return i2cWriteReadBufferEx (address, buffer, putLength, getLength, i2cTimeoutInTicks);
}

int i2cWriteBufferPoll (U8 address, U8 *buffer, U32 bufferLength)
{
  int r;

  if (!(r = i2cKickOff (address, buffer, bufferLength, 0, ACKPOLL_NO)))
    if (!(r = i2cWaitComplete (i2cTimeoutInTicks)))
      r = i2cPoll (address);

  return r;
}

int i2cWriteReadBufferPoll (U8 address, U8 *buffer, U32 putLength, U32 getLength)
{
  int r;

  if (!(r = i2cKickOff (address, buffer, putLength, getLength, ACKPOLL_NO)))
    if (!(r = i2cWaitComplete (i2cTimeoutInTicks)))
      r = i2cPoll (address);

  return r;
}
