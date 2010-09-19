//
//  $Id: iap.c 188 2008-10-19 14:36:39Z jcw $
//  $Revision: 188 $
//  $Author: jcw $
//  $Date: 2008-10-19 10:36:39 -0400 (Sun, 19 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/iap/iap.c $
//

#include "FreeRTOS.h"

#include <string.h>
#include "../cpu/cpu.h"
#include "iap.h"

//
//
//
#define CPUCLK_IN_KHZ 48000

#define IRQ_MASK  0x00000080
#define FIQ_MASK  0x00000040
#define INTs_MASK (IRQ_MASK | FIQ_MASK)

//
//
//
typedef struct flashSectorToAddress_s
{
  unsigned long address;
  int sizeInBytes;
}
flashSectorToAddress_t;

static flashSectorToAddress_t flashSectorToAddress [] = 
{
  { 0x00000000, 4096 },   // 0
  { 0x00001000, 4096 },   // 1
  { 0x00002000, 4096 },   // 2
  { 0x00003000, 4096 },   // 3
  { 0x00004000, 4096 },   // 4
  { 0x00005000, 4096 },   // 5
  { 0x00006000, 4096 },   // 6
  { 0x00007000, 4096 },   // 7
  { 0x00008000, 32768 },  // 8
  { 0x00010000, 32768 },  // 9
  { 0x00018000, 32768 },  // 10
  { 0x00020000, 32768 },  // 11
  { 0x00028000, 32768 },  // 12
  { 0x00030000, 32768 },  // 13
  { 0x00038000, 32768 },  // 14
  { 0x00040000, 32768 },  // 15
  { 0x00048000, 32768 },  // 16
  { 0x00050000, 32768 },  // 17
  { 0x00058000, 32768 },  // 18
  { 0x00060000, 32768 },  // 19
  { 0x00068000, 32768 },  // 20
  { 0x00070000, 32768 },  // 21
  { 0x00078000, 4096 },   // 22
  { 0x00079000, 4096 },   // 23
  { 0x0007a000, 4096 },   // 24
  { 0x0007b000, 4096 },   // 25
  { 0x0007c000, 4096 },   // 26
};

typedef struct iapErrnoStr_s
{
  int errno;
  const char *text;
}
iapErrnoStr_t;

static iapErrnoStr_t iapErrnoStr [] =
{
  { IAP_RESULT_CMD_SUCCESS,         "success" },
  { IAP_RESULT_INVALID_COMMAND,     "invalid command" },
  { IAP_RESULT_SRC_ADDR_ERROR,      "source address error" },
  { IAP_RESULT_DST_ADDR_ERROR,      "destination address error" },
  { IAP_RESULT_SRC_ADDR_NOT_MAPPED, "source address not mapped" },
  { IAP_RESULT_DST_ADDR_NOT_MAPPED, "destination address not mapped" },
  { IAP_RESULT_COUNT_ERROR,         "count error" },
  { IAP_RESULT_INVALID_SECTOR,      "invalid sector" },
  { IAP_RESULT_SECTOR_NOT_BLANK,    "sector not blank" },
  { IAP_RESULT_SECTOR_NOT_PREPARED, "sector not prepared" },
  { IAP_RESULT_COMPARE_ERROR,       "compare error" },
  { IAP_RESULT_BUSY,                "busy" },
  { IAP_RESULT_PARAM_ERROR,         "parameter error" },
  { IAP_RESULT_ADDR_ERROR,          "address error" },
  { IAP_RESULT_ADDR_NOT_MAPPED,     "address not mapped" },
  { IAP_RESULT_CMD_LOCKED,          "command locked" },
  { IAP_RESULT_INVALID_CODE,        "invalid code" },
  { IAP_RESULT_INVALID_BAUD_RATE,   "invalid baud rate" },
  { IAP_RESULT_ANVALID_STOP_BIT,    "invalid stop bit" },
  { IAP_RESULT_CRP_ENABLED,         "CRP enabled" },
  { IAP_RESULT_X_NOTSAFEREGION,     "sector or address not in safe region" },
  { IAP_RESULT_X_NOSAFEREGIONAVAIL, "no safe sectors available (all of memory used?)" },
};

//
//
//
static unsigned int iapCommands [5];
static unsigned int iapResults [2];
static int iapErrno = 0;
extern unsigned long __end_of_text__;
static unsigned int end_of_text = (unsigned int) &__end_of_text__;


//
//
//
void iapInit (void)
{
}

//
//
//
int iapSectorToAddress (int sectorNumber, unsigned long *address, int *sectorSize)
{
  if (sectorNumber >= (int) arrsizeof (flashSectorToAddress))
    return -1;

  if (address)
    *address = flashSectorToAddress [sectorNumber].address;
  if (sectorSize)
    *sectorSize = flashSectorToAddress [sectorNumber].sizeInBytes;

  return 0;
}

//
//  Convert address to sector, or -1 if address not in flash area
//
int iapAddressToSector (unsigned long address)
{
  int i;

  for (i = 0; i < (int) arrsizeof (flashSectorToAddress); i++)
    if (address < (flashSectorToAddress [i].address + flashSectorToAddress [i].sizeInBytes))
      return i;

  iapErrno = IAP_RESULT_INVALID_SECTOR;
  return -1;
}

//
//  1 == address in safe region, 0 if not
//
int iapIsSafeAddress (unsigned long address)
{
  int eotSector;
  int addressSector;

  if ((eotSector = iapAddressToSector (end_of_text)) == -1)
    return 0;
  if ((addressSector = iapAddressToSector (address)) == -1)
    return 0;

  if (addressSector <= eotSector)
  {
    iapErrno = IAP_RESULT_X_NOTSAFEREGION;
    return 0;
  }

  return 1;
}

//
//  0 == not safe sector, 1 == safe (not in a code sector)
//
int iapIsSafeSector (int sector)
{
  int eotSector;

  if ((eotSector = iapAddressToSector (end_of_text)) == -1)
    return 0;

  if (sector <= eotSector)
  {
    iapErrno = IAP_RESULT_X_NOTSAFEREGION;
    return 0;
  }

  return 1;
}

//
//  Returns a safe sector number or -1 if none available
//
int iapFindSafeSector (void)
{
  unsigned int i;

  for (i = 0; i < arrsizeof (flashSectorToAddress); i++)
    if (iapIsSafeSector (i))
      return i;

  iapErrno = IAP_RESULT_X_NOSAFEREGIONAVAIL;
  return -1;
}

//
//  1 == sector is safe, 0 == not safe
//
int iapIsValidSector (int sector)
{
  if ((sector < 0) || (sector >= (int) arrsizeof (flashSectorToAddress)))
  {
    iapErrno = IAP_RESULT_INVALID_SECTOR;
    return 0;
  }

  return 1;
}

//
//
//
int iapGetErrno (void)
{
  return iapErrno;
}

const char *iapStrerror (int err)
{
  unsigned int i;

  for (i = 0; i < arrsizeof (iapErrnoStr); i++)
    if (iapErrnoStr [i].errno == err)
      return iapErrnoStr [i].text;

  return NULL;
}

//
//
//
static void iapCall (void) __attribute ((naked));
static void iapCall (void)
{
  register void *pFP0 asm("r0") = iapCommands;
  register void *pFP1 asm("r1") = iapResults;

  asm volatile(" bx  %[iapLocation]"
               :
               : "r" (pFP0), "r" (pFP1), [iapLocation] "r" (IAP_LOCATION) );
}

//
//
//
static inline unsigned __get_cpsr (void)
{
  unsigned long retval;

  asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );

  return retval;
}

static inline void __set_cpsr (unsigned val)
{
  asm volatile (" msr  cpsr, %0" : /* no outputs */ : "r" (val)  );
}

static unsigned disableInts (void)
{
  unsigned _cpsr;

  _cpsr = __get_cpsr ();

  do
    __set_cpsr (_cpsr | INTs_MASK);
  while ((__get_cpsr () ^ INTs_MASK) & INTs_MASK);

  return _cpsr;
}

static unsigned restoreInts (unsigned oldCPSR)
{
  unsigned _cpsr;

  _cpsr = __get_cpsr();

  do
    __set_cpsr ((_cpsr & ~INTs_MASK) | (oldCPSR & INTs_MASK));
  while ((__get_cpsr () ^ oldCPSR) & INTs_MASK);

  return _cpsr;
}

//
//
//
int iapPrepareSectors (int startingSector, int endingSector)
{
  unsigned int cpsr;

  if (!iapIsSafeSector (startingSector) || !iapIsSafeSector (endingSector))
    return -1;

  iapCommands [0] = IAP_CMD_PREPARE;
  iapCommands [1] = startingSector;
  iapCommands [2] = endingSector;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  return ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS) ? 0 : -1;
}

//
//  IAP_CMD_COPYRAMTOFLASH can span multiple sectors (2, at any rate, since bufferLen
//  must be 256, 512, 1024 or 4096, and the smallest sectors are 4096 bytes).  Although
//  more than 2 sectors can be prepared for writing, it's useless to do so, since
//  after each IAP_CMD_COPYRAMTOFLASH, the sectors are re-locked.
//
int iapWriteSectors (unsigned int address, unsigned char *buffer, int bufferLen)
{
  unsigned int cpsr;

  iapCommands [0] = IAP_CMD_COPYRAMTOFLASH;
  iapCommands [1] = address;
  iapCommands [2] = (int) buffer;
  iapCommands [3] = bufferLen;
  iapCommands [4] = CPUCLK_IN_KHZ;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  if ((iapErrno = iapResults [0]) != IAP_RESULT_CMD_SUCCESS)
    return -1;

  return 0;
}

int iapFillSectors (int startingSector, int endingSector, int byte)
{
  int sector;
  unsigned char buffer [256];

  if (!iapIsSafeSector (startingSector) || !iapIsSafeSector (endingSector))
    return -1;

  memset (buffer, byte, sizeof (buffer));

  for (sector = startingSector; sector <= endingSector; sector++)
  {
    int i;
    unsigned long address;
    int sectorSize;

    if (iapSectorToAddress (sector, &address, &sectorSize))
      return -1;

    for (i = 0; i < sectorSize; i += sizeof (buffer))
    {
      if (iapPrepareSectors (sector, sector) == -1)
        return -1;
      if (iapWriteSectors (address + i, buffer, sizeof (buffer)) == -1)
        return -1;
      if (iapCompare (address + i, buffer, sizeof (buffer)) == -1)
        return -1;
    }
  }

  return 0;
}

int iapEraseSectors (int startingSector, int endingSector)
{
  unsigned int cpsr;

  if (!iapIsSafeSector (startingSector) || !iapIsSafeSector (endingSector))
    return -1;

  if (iapPrepareSectors (startingSector, endingSector) == -1)
    return -1;

  iapCommands [0] = IAP_CMD_ERASE;
  iapCommands [1] = startingSector;
  iapCommands [2] = endingSector;
  iapCommands [3] = CPUCLK_IN_KHZ;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  return ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS) ? 0 : -1;
}

//
// -1 = error (iapErrno set)
//  0 = sector blank
//  1 = sector not blank
//
int iapBlankCheckSectors (int startingSector, int endingSector)
{
  unsigned int cpsr;

  if (!iapIsValidSector (startingSector) || !iapIsValidSector (endingSector))
    return -1;

  iapCommands [0] = IAP_CMD_BLANKCHECK;
  iapCommands [1] = startingSector;
  iapCommands [2] = endingSector;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  if ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS)
    return 0;
  if (iapResults [0] == IAP_RESULT_SECTOR_NOT_BLANK)
    return 1;

  return -1;
}

//
//
//
unsigned int iapReadPartID (void)
{
  unsigned int cpsr;

  iapCommands [0] = IAP_CMD_READPARTID;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  return iapResults [1];
}

//
//
//
unsigned int iapReadBootCodeVersion (void)
{
  unsigned int cpsr;

  iapCommands [0] = IAP_CMD_READBOOTCODEVER;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  return iapResults [1];
}

//
//
//
int iapCompare (unsigned int address, unsigned char *buffer, int bufferLen)
{
  unsigned int cpsr;

  iapCommands [0] = IAP_CMD_COMPARE;
  iapCommands [1] = address;
  iapCommands [2] = (int) buffer;
  iapCommands [3] = bufferLen;

  cpsr = disableInts ();

  iapCall ();

  restoreInts (cpsr);

  return ((iapErrno = iapResults [0]) == IAP_RESULT_CMD_SUCCESS) ? 0 : -1;
}

//
//
//
void iapISP (void)
{
  int i;
  REG32 *p;

  portDISABLE_INTERRUPTS ();

  VIC_IntEnClr = VIC_IntEnClr_MASK;

  SCB_PLLCON = 0;
  SCB_PLLCFG = 0;
  SCB_PLLFEED = SCB_PLLFEED_FEED1;
  SCB_PLLFEED = SCB_PLLFEED_FEED2;

  USB_PLLCON = 0; 
  USB_PLLCFG = 0; 
  USB_PLLFEED = USB_PLLFEED_FEED1;
  USB_PLLFEED = USB_PLLFEED_FEED2;

  for (p = (REG32 *) T1_BASE_ADDR, i = 0; i< 0x74 / 4; i++)
    *p++ = 0;

  T1_IR = 0xff;

  SCB_PCONP = SCB_PCONP_ATRESET;

  UART0_IER = 0x00; 
  UART0_LCR = 0x80; 
  UART0_DLL = 0x01; 
  UART0_DLM = 0x00;
  UART0_LCR = 0x00; 
  UART0_FCR = 0x06; 
  UART0_FDR = 0x10;

  SCB_MEMMAP = 0;
  SCB_VPBDIV = 0;

  PCB_PINSEL0 = PCB_PINSEL0_ALL_GPIO;
  PCB_PINSEL1 = PCB_PINSEL1_ALL_GPIO;
  
  GPIO0_FIODIR = 0x00004000;
  GPIO0_FIOCLR = 0x00004000;

  WD_TC = 0x0ffffff; 
  WD_MOD = 0;
  WD_FEED = WD_FEED_FEED1;
  WD_FEED = WD_FEED_FEED2;

  iapCommands [0] = IAP_CMD_REINVOKEISP;

  // disableInts ();
  // cpuPLLDisable ();
  // cpuT1Disable ();

  iapCall ();
}
