//
//  $Id: mmc.c 96 2008-10-10 15:29:59Z jcw $
//  $Revision: 96 $
//  $Author: jcw $
//  $Date: 2008-10-10 11:29:59 -0400 (Fri, 10 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/mmc/mmc.c $
//

/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2005
 *
 *    File name   : mmc.c
 *    Description : MMC
 *
 *    History :
 *    1. Data        : July 1, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 96 $
 *
 * (C) Joel Winarske, 2006,2007                                        
**************************************************************************/

#include <stdio.h>

#include "mmc.h"
#include "ssp.h"

#define MMC_RET_ERROR(Res)          do { mmcLastError = Res; return (MmcCardError); } while (0)
#define MMC_RET_DATA_ERR(Res)       do { mmcLastError = Res; return (MmcDataError); } while (0)

#define CSD_GET_TRAN_SPEED_EXP()      (mmcSdCsd [ 3] & 0x07)
#define CSD_GET_TRAN_SPEED_MANT()    ((mmcSdCsd [ 3] & 0xF8) >> 3)
#define CSD_GET_NSAC()                (mmcSdCsd [ 2])
#define CSD_GET_TAAC_EXP()            (mmcSdCsd [ 1] & 0x7)
#define CSD_GET_TAAC_MANT()          ((mmcSdCsd [ 1] & 0xF8) >> 3)
#define CSD_GET_R2W_FACTOR()         ((mmcSdCsd [12] & 0x1C) >> 2)
#define CSD_GET_READ_BL_LEN()         (mmcSdCsd [ 5] & 0x0F)
#define CSD_GET_C_SIZE()            (((mmcSdCsd [ 6] & 0x03) << 10) + (mmcSdCsd [7] << 2) + ((mmcSdCsd [8] & 0xc0) >> 6))
#define CSD_GET_C_SIZE_MULT()       (((mmcSdCsd [ 9] & 0x03) << 1) + ((mmcSdCsd [10] & 0x80) >> 7))
#define CSD_GET_PERM_WRITE_PROTECT() ((mmcSdCsd [14] & 0x20) >> 5)
#define CSD_GET_TMP_WRITE_PROTECT()  ((mmcSdCsd [14] & 0x10) >> 4)

static const U32 mmcTransfExp [] =
{
     10000UL,
    100000UL,
   1000000UL,
  10000000UL,
         0UL,
         0UL,
         0UL,
         0UL,
};

static const U32 mmmcAccessTime [] =
{
 10000000UL,
  1000000UL,
   100000UL,
    10000UL,
     1000UL,
      100UL,
       10UL,
        1UL,
};

static const U32 mmcCsdMant [] =
{
  0UL,10UL,12UL,13UL,15UL,
  20UL,25UL,
  30UL,35UL,
  40UL,45UL,
  50UL,55UL,
  60UL,
  70UL,
  80UL,
};

static const U32 mmcAccessTimeMant [] =
{
  0UL,100UL,83UL,77UL,67UL,
  50UL,40UL,
  33UL,29UL,
  25UL,22UL,
  20UL,18UL,
  17UL,
  14UL,
  13UL,
};

static const mmcCommands_t mmcCmd [CMD_END] =
{
  { 0x40, MmcNoArg,     MmcR1 }, // CMD0
  { 0x41, MmcNoArg,     MmcR1 }, // CMD1
  { 0x49, MmcNoArg,     MmcR1 }, // CMD9
  { 0x4A, MmcNoArg,     MmcR1 }, // CMD10
  { 0x4C, MmcNoArg,     MmcR1 }, // CMD12
  { 0x4D, MmcNoArg,     MmcR2 }, // CMD13
  { 0x50, MmcBlockLen,  MmcR1 }, // CMD16
  { 0x51, MmcDataAdd,   MmcR1 }, // CMD17
  { 0x52, MmcDataAdd,   MmcR1 }, // CMD18
  { 0x58, MmcDataAdd,   MmcR1 }, // CMD24
  { 0x59, MmcDataAdd,   MmcR1 }, // CMD25
  { 0x5B, MmcNoArg,     MmcR1 }, // CMD27
  { 0x5C, MmcDataAdd,   MmcR1b}, // CMD28
  { 0x5D, MmcDataAdd,   MmcR1b}, // CMD29
  { 0x5E, MmcDataAdd,   MmcR1 }, // CMD30
  { 0x60, MmcDataAdd,   MmcR1 }, // CMD32
  { 0x61, MmcDataAdd,   MmcR1 }, // CMD33
  { 0x62, MmcDataAdd,   MmcR1 }, // CMD34
  { 0x63, MmcDataAdd,   MmcR1 }, // CMD35
  { 0x64, MmcDataAdd,   MmcR1 }, // CMD36
  { 0x65, MmcDataAdd,   MmcR1 }, // CMD37
  { 0x66, MmcDummyWord, MmcR1b}, // CMD38
  { 0x6A, MmcDummyWord, MmcR1b}, // CMD42
  { 0x77, MmcNoArg,     MmcR1 }, // CMD55
  { 0x78, MmcNoArg,     MmcR1 }, // CMD56
  { 0x7A, MmcNoArg,     MmcR3 }, // CMD58
  { 0x7B, MmcDummyWord, MmcR1 }, // CMD59
  { 0x69, MmcNoArg,     MmcR1 } // ACMD41
};

static mmcDiskStatus_t mmcDiskStatus;
static U32 mmcLastError; 
static U32 Tnac;
static U32 Twr;
static U8 mmcSdCsd [16];

//
//
//
static U32 mmcSendCmd (mmcSpiCmdInd_t commandIndex, U32 arg);
static U32 mmcSetBlockLen (U32 length);
static void mmcCSDImplement (void);

/*************************************************************************
 * Function Name: mmcSendCmd
 * Parameters: MmcSpiCmdInd_t commandIndex,U32 arg
 *
 * Return: U32
 *
 * Description: MMC commands implemet
 *
 *************************************************************************/
static U32 mmcSendCmd (mmcSpiCmdInd_t commandIndex, U32 arg)
{
  U32 ch = 0xff;
  U32 i;

  //
  //  Chip Select
  //
  sspChipSelect (1);

  //
  //  Send command code
  //
  sspTransferByte (mmcCmd [commandIndex].txData);

  //
  //  Send command's arguments
  //
  if (mmcCmd [commandIndex].arg == MmcNoArg)
  {
    sspTransferByte (0x00);
    sspTransferByte (0x00);
    sspTransferByte (0x00);
    sspTransferByte (0x00);
  }
  else
  {
    sspTransferByte (arg >> 24);
    sspTransferByte (arg >> 16);
    sspTransferByte (arg >> 8);
    sspTransferByte (arg);
  }

  //
  //  Send CRC
  //
  if (commandIndex == CMD0)
    sspTransferByte (0x95);
  else
    sspTransferByte (0xff);

  for (i = 9; i && (ch == 0xff); --i) 
    ch = sspTransferByte (0xff);

  if (i == 0)
  {
    sspChipSelect (0);
    return ((U32) -1);
  }

  switch (mmcCmd [commandIndex].resp)
  {
    case MmcR1b :
      {
        U32 busy;

        for (busy = 0, i = Twr; i && (busy != 0xff); --i)
          busy = sspTransferByte (0xff);
      }
      return (ch);

    case MmcR1 :
      return (ch);

    case MmcR2 :
      arg  = ((U32) ch << 8) & 0x0000ff00;
      arg |= sspTransferByte (0xff) & 0xff;
      return (arg);

    case MmcR3 :
    default:
      arg  = ((U32) ch << 24) & 0xff000000;
      arg |= ((U32) sspTransferByte (0xff) << 16) & 0x00ff0000;
      arg |= ((U32) sspTransferByte (0xff) << 8 ) & 0x0000ff00;
      arg |= sspTransferByte (0xff) & 0xff;
      return (arg);
  }
}

/*************************************************************************
 * Function Name: mmcSetBlockLen
 * Parameters: U32 length
 *
 * Return: U32
 *
 * Description: Set Block length Return command's result
 *
 *************************************************************************/
static U32 mmcSetBlockLen (U32 length)
{
  U32 res = mmcSendCmd (CMD16, length);
  sspChipSelect (0);
  return (res);
}

/*************************************************************************
 * Function Name: mmcCSDImplement
 * Parameters:  none
 *
 * Return: none
 *
 * Description: Implemet data from CSD
 *
 *************************************************************************/
static void mmcCSDImplement (void)
{
  U32 frequency;

  //
  // Calculate SPI max clock
  //
  frequency = mmcTransfExp [CSD_GET_TRAN_SPEED_EXP ()] * mmcCsdMant [CSD_GET_TRAN_SPEED_MANT ()];

  if (frequency > 20000000)
    frequency = 20000000;

  frequency = sspSetClockFreq (frequency);

  if (mmcDiskStatus.diskType == DISKTYPE_MMC)
  {
    Tnac = mmmcAccessTime [CSD_GET_TAAC_EXP ()] * mmcAccessTimeMant [CSD_GET_TAAC_MANT ()];
    Tnac = frequency / Tnac;
    Tnac += 1 << (CSD_GET_NSAC () + 4);
    Tnac *= 10;
    Twr   = Tnac * CSD_GET_R2W_FACTOR ();
  }
  else
  {
    Tnac = frequency / SD_READ_TIME_OUT;
    Twr  = frequency / SD_WRITE_TIME_OUT;
  }

  mmcDiskStatus.blockSize = 1 << CSD_GET_READ_BL_LEN ();
  mmcDiskStatus.blockNumber = (CSD_GET_C_SIZE () + 1) * (4 << CSD_GET_C_SIZE_MULT ());
  mmcDiskStatus.writeProtect = sspHardwareWriteProtected () | CSD_GET_PERM_WRITE_PROTECT () | CSD_GET_TMP_WRITE_PROTECT ();
}

//
//
//
BOOL mmcWriteProtect (void)
{
  return mmcDiskStatus.writeProtect;
}

BOOL mmcWaitReady (void)
{
  return sspWaitReady () == 0xff;
}

BOOL mmcCardPresent (void)
{
  return sspHardwareCardPresent ();
}

/*************************************************************************
 * Function Name: mmcReadCardInfo
 * Parameters: U8 *pData,
 *             mmcSpiCmdInd_t command - CMD9, CMD10 are only allowed
 *
 * Return: mmcState_t
 *
 * Description: Read CSD or CID  registers depending on command
 *
 *************************************************************************/
mmcState_t mmcReadCardInfo (U8 *pData, mmcSpiCmdInd_t command)
{
  U32 i;
  U32 res;

  switch (command)
  {
    case CMD9 :
    case CMD10 :
      break;

    default:
      return (MmmcParameterError);
  }

  sspChipSelect (0);
  sspTransferByte (0xff);

  if ((res = mmcSendCmd (command, 0)) == MMC_OK)
  {
    for (i = 8; i ; --i)
    {
      if (((res = sspTransferByte (0xff)) | MMC_DATA_ERR_TOKEN) == MMC_DATA_ERR_TOKEN)
        MMC_RET_DATA_ERR (res);
      else if (res == MMC_DATA_TOKEN)
      {
        for (i = 0; i <16 ; ++i)
          *pData++ = sspTransferByte (0xff);

        //
        // CRC receive
        //
        sspTransferByte (0xff);
        sspTransferByte (0xff);
        sspChipSelect (0);
        return (MmcOk);
      }
    }
  }

  sspChipSelect (0);
  MMC_RET_ERROR (res);
}

/*************************************************************************
 * Function Name: mmcRead
 * Parameters: U8 *pData, U32 address, U32 length
 *
 * Return: mmcState_t
 *
 * Description: Read from a MMC
 *
 *************************************************************************/
mmcState_t mmcRead (U8 *pData, U32 address, U32 length)
{
  U32 res;
  U32 i;

  //
  //  Sector number is what is actually passed in, along with the number
  //  of sectors.  Convert this to an address and length, in bytes.
  //
  address *= mmcDiskStatus.blockSize;
  length *= mmcDiskStatus.blockSize;

  //
  //  For synchronization
  //
  sspChipSelect (0);
  sspTransferByte (0xff);
  res = mmcSendCmd (CMD17, address);

  if (res == MMC_OK)
  {
    for (i = Tnac; i; --i)
    {
      res = sspTransferByte (0xff);

      if ((res | MMC_DATA_ERR_TOKEN) == MMC_DATA_ERR_TOKEN)
        MMC_RET_DATA_ERR (res);
      else if (res == MMC_DATA_TOKEN)
      {
        sspReceiveBlock (pData, length);

        //
        //  CRC receive
        //
        sspTransferByte (0xff);
        sspTransferByte (0xff);
        sspChipSelect (0);
        return (MmcOk);
      }
    }

    sspChipSelect (0);
    return (MmcNoResponse);
  }

  sspChipSelect (0);
  MMC_RET_ERROR (res);
}

/*************************************************************************
 * Function Name: mmcWrite
 * Parameters: U8 *pData, U32 address, U32 length
 *
 * Return: mmcState_t
 *
 * Description: Write to a MMC
 *
 *************************************************************************/
mmcState_t mmcWrite (const U8 *pData, U32 address, U32 length)
{
  U32 res;
  U32 i;

  //
  //  Sector number is what is actually passed in, along with the number
  //  of sectors.  Convert this to an address and length, in bytes.
  //
  address *= mmcDiskStatus.blockSize;
  length *= mmcDiskStatus.blockSize;

  //
  //  For synchronization
  //
  sspChipSelect (0);
  sspTransferByte (0xff);

  if ((res = mmcSendCmd (CMD24, address)) == MMC_OK)
  {
    sspTransferByte (0xff);
    sspTransferByte (MMC_DATA_TOKEN);
    sspSendBlock (pData, length);

    //
    //  CRC Send
    //
    sspTransferByte (0xff);
    sspTransferByte (0xff);

    if ((sspTransferByte (0xff) & 0x1F) != 0x05)
      MMC_RET_ERROR (res);

    for (i = Twr; i ;i--)
      if (sspTransferByte (0xff) == 0xff)
        break;
    
    sspChipSelect (0);

    if (i == 0)
      return (MmcNoResponse);

    return (MmcOk);
  }

  sspChipSelect (0);
  MMC_RET_ERROR (res);
}

/*************************************************************************
 * Function Name: mmcVerify
 * Parameters: U8 *pData, U32 address, U32 length
 *
 * Return: mmcState_t
 *
 * Description: Verify on a MMC
 *
 *************************************************************************/
mmcState_t mmcVerify (U8 *pData, U32 address, U32 length)
{
  U32 res,i;

  //
  //  For synchronization
  //
  sspChipSelect (0);
  sspTransferByte (0xff);

  if ((res = mmcSendCmd (CMD17, address)) == MMC_OK)
  {
    for (i = Tnac; i; --i)
    {
      res = sspTransferByte (0xff);

      if ((res | MMC_DATA_ERR_TOKEN) == MMC_DATA_ERR_TOKEN)
        MMC_RET_DATA_ERR (res);
      else if (res == MMC_DATA_TOKEN)
      {
        for (res = 0, i = 0; i < length; ++i, ++pData)
        {
          *pData ^= sspTransferByte (0xff);

          if (*pData != 0)
            res = 1;
        }

        //
        //  CRC receive
        //
        sspTransferByte (0xff);
        sspTransferByte (0xff);
        sspChipSelect (0);
        sspTransferByte (0xff);
        sspTransferByte (0xff);

        if (res)
          return (MmcMiscompare);

        return (MmcOk);
      }
    }

    return (MmcNoResponse);
  }

  MMC_RET_ERROR (res);
}

/*************************************************************************
 * Function Name: mmcInitMedia
 * Parameters: none
 *
 * Return: mmcState_t
 *
 * Description: MMC detect and initialize
 *
 *************************************************************************/
mmcState_t mmcInitMedia (void)
{
  U32 i;
  U32 res;

  mmcDiskStatus.blockNumber = mmcDiskStatus.blockSize = mmcLastError = 0;
  Tnac = 1;

  sspInit ();

  if (!sspHardwareCardPresent ())
    return (MmcNoPresent);

  //
  //  Clock Freq. Identification Mode < 400kHz
  //
  sspSetClockFreq (IDENTIFICATIONMODECLOCK);

  //
  //  Set maximum time out
  //
  Tnac = IDENTIFICATIONMODECLOCK / SD_READ_TIME_OUT;

  //
  //  Power up cycles.  After power up at least 74 clock cycles are required 
  //  prior to starting bus communication
  //
  for (i = 0; i < 2; i++)
  {
    sspChipSelect (0);

    for (res = 10; res; --res)
      sspTransferByte (0xff);

    //
    //  CMD0 (Go to IDLE) to put MMC in SPI mode
    //
    res = mmcSendCmd (CMD0, 0);
    sspChipSelect (0);

    if (res == MMC_IDLE_STATE)
      break;
  }

  if (res != MMC_IDLE_STATE)
    return (MmcNoResponse);

  //
  //  Determine card type as SD or MMC
  //
  mmcDiskStatus.diskType = DISKTYPE_MMC;

  for (i = 100; i; --i)
  {
    sspChipSelect (0);
    sspTransferByte (0xff);
    res = mmcSendCmd (CMD55, 0);
    sspChipSelect (0);
    sspChipSelect (0);
    sspTransferByte (0xff);
    res = mmcSendCmd (ACMD41, 0);
    sspChipSelect (0);

    if (res & MMC_ILLEGAL_CMD)
    {
      //
      //  MMC card may be CMD1 for MMC Init sequence will be complete within 500ms
      //
      for (i = 100; i; --i)
      {
        sspChipSelect (0);
        sspTransferByte (0xff);
        res = mmcSendCmd (CMD1, 0);
        sspChipSelect (0);

        if (res == MMC_OK)
          break;

        sspDelay1ms (50);
      }
      break;
    }

    if (res == MMC_OK)
    {
      mmcDiskStatus.diskType = DISKTYPE_SD;
      break;
    }

    sspDelay1ms (50);
  }

  if (i == 0)
    return (MmcNoResponse);

  //
  //  Read CSD.  CSD must always be valid
  //
  if ((res = mmcReadCardInfo (mmcSdCsd, CMD9)) != MmcOk)
    return (MmcNoResponse);

  //
  //  Implement CSD data, and set block size
  //
  mmcCSDImplement ();
  mmcSetBlockLen (mmcDiskStatus.blockSize);

#if 0
  mmcDecodeCSD (mmcSdCsd);
#endif

  return (MmcOk);
}

void mmcDumpCSD (void)
{
#ifdef CFG_MMCDUMP
  U8 mmcCSD [16];

  if (mmcReadCardInfo (mmcCSD, CMD9) == MmcOk)
  {
    U8 i;

    for (i = 0; i < arrsizeof (mmcCSD); i++)
      printf (" %02X", mmcCSD [i]);

    putchar (' ');

    for (i = 0; i < arrsizeof (mmcCSD); i++) 
    {
      if ((mmcCSD [i] < 0x20) || (mmcCSD [i] >= 0x7F))
        putchar ('.');
      else
        putchar (mmcCSD [i]);
    }
  }
  else
    printf ("CSD read error\n");
#else
  printf ("Not implemented\n");
#endif
}

typedef struct csd_11_s
{
  unsigned int not_used_1          : 2;   // 0..1  [121:120]   0x44
  unsigned int mmc_prot            : 4;   // 2..5  [125:122]
  unsigned int csd_structure       : 2;   // 6..7  [127:126]

  unsigned int taac_exp            : 3;   // 0..2  [114:112]   0x26
  unsigned int taac_value          : 4;   // 3..6  [118:115]
  unsigned int taac_rsvd           : 1;   // 7..7  [119:119]

  unsigned int nsac                : 8;   // 0..7  [111:104]   0x00

  unsigned int tran_speed_exp      : 3;   // 0..2  [98:96]     0x2a
  unsigned int tran_speed_value    : 4;   // 3..6  [102:99]
  unsigned int tran_speed_rsvd     : 1;   // 7..7  [103:103]

  unsigned int ccc_hi              : 8;   // 0..7  [95:88]     0x1f

  unsigned int read_bl_len         : 4;   // 0..3  [83:80]     0xf9
  unsigned int ccc_lo              : 4;   // 4..7  [87:84]

  unsigned int c_size_hi           : 2;   // 0..1  [73:72]     0x83
  unsigned int not_used_2          : 2;   // 2..3  [75:74]
  unsigned int dsr_imp             : 1;   // 4..4  [76:76]
  unsigned int read_blk_misalign   : 1;   // 5..5  [77:77]
  unsigned int write_blk_misalign  : 1;   // 6..6  [78:78]
  unsigned int read_bl_partial     : 1;   // 7..7  [79:79]

  unsigned int c_size_mid          : 8;   // 0..7  [71:64]     0xd3

  unsigned int vdd_r_curr_max      : 3;   // 0..1  [58:56]     0xe3
  unsigned int vdd_r_curr_min      : 3;   // 2..5  [61:59]
  unsigned int c_size_lo           : 2;   // 6..7  [63:62]

  unsigned int c_size_mult_hi      : 2;   // 0..1  [49:48]     0x91
  unsigned int vdd_w_curr_max      : 3;   // 2..4  [52:50]
  unsigned int vdd_w_curr_min      : 3;   // 4..7  [55:53]

  unsigned int erase_grp_size_lo   : 2;   // 0..1  [41:40]     0x83
  unsigned int erase_grp_mult      : 5;   // 2..6  [46:42]
  unsigned int c_size_mult_lo      : 1;   // 7..7  [47:47]

  unsigned int wp_grp_size         : 5;   // 0..4  [36:32]     0xff
  unsigned int erase_grp_size_hi   : 3;   // 5..7  [39:37]

  unsigned int write_bl_len_hi     : 2;   // 0..1  [25:24]     0x92
  unsigned int r2w_factor          : 3;   // 2..4  [28:26]
  unsigned int default_ecc         : 2;   // 5..6  [30:29]

  unsigned int wp_grp_enable       : 1;   // 7..7  [31:31]     
  unsigned int not_used_3          : 5;   // 0..4  [20:16]     0x40

  unsigned int write_bl_partial    : 1;   // 5..5  [21:21]
  unsigned int write_bl_len_lo     : 2;   // 6..7  [23:22]

  unsigned int ecc                 : 2;   // 0..1  [9:8]       0x40
  unsigned int not_used_5          : 2;   // 2..3  [11:10]
  unsigned int tmp_write_protect   : 1;   // 4..4  [12:12]
  unsigned int perm_write_protect  : 1;   // 5..5  [13:13]
  unsigned int copy                : 1;   // 6..6  [14:14]
  unsigned int not_used_4          : 1;   // 7..7  [15:15]

  unsigned int notused_6           : 1;   // 0..0  [0:0]       0x67
  unsigned int crc                 : 7;   // 1..7  [7:1]
}
__attribute__ ((packed)) csd_11_t;

#ifdef CFG_MMCDECODER
static const unsigned int tran_exp [] = 
{
  10000,    100000,   1000000,  10000000,
  0,        0,        0,        0
};

static const unsigned char tran_mant [] = 
{
  0,  10, 12, 13, 15, 20, 25, 30,
  35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned int taac_exp [] = 
{
  1,  10, 100,  1000, 10000,  100000, 1000000, 10000000,
};

static const unsigned int taac_mant [] = 
{
  0,  10, 12, 13, 15, 20, 25, 30,
  35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned int vdd_values [] =
{
  500, 1000, 5000, 10000, 25000, 35000, 60000, 100000
};
#endif

void mmcDecodeCSD (void)
{
#ifdef CFG_MMCDECODER
  U8 mmcCSD [16];

  csd_11_t *p = (csd_11_t *) mmcCSD;

  if (mmcReadCardInfo (mmcCSD, CMD9) == MmcOk)
  {
    printf ("\n");
    printf ("sizeof (csd_11_t)     : %lu\n",        sizeof (csd_11_t));
    printf ("CSD structure version : 1.%d\n",       p->csd_structure);
    printf ("MMC protocol version  : %d\n",         p->mmc_prot);
    printf ("TAAC                  : %d\n",         (taac_exp [p->tran_speed_exp] * taac_mant [p->tran_speed_value] + 9) / 10);
    printf ("NSAC                  : %d\n",         p->nsac * 100);
    printf ("TRAN_SPEED            : %d Mhz\n",     (tran_exp [p->tran_speed_exp] * tran_mant [p->tran_speed_value]));
    printf ("CCC                   : 0x%03x\n",     (p->ccc_hi << 4) | p->ccc_lo);
    printf ("READ_BL_LEN           : %d (%d)\n",    p->read_bl_len, 1 << p->read_bl_len);
    printf ("READ_BL_PARTIAL       : %d\n",         p->read_bl_partial);
    printf ("WRITE_BL_LEN          : %d (%d)\n",    (p->write_bl_len_hi << 2) | p->write_bl_len_lo, 1 << ((p->write_bl_len_hi << 2) | p->write_bl_len_lo));
    printf ("WRITE_BL_PARTIAL      : %d\n",         p->write_bl_partial);
    printf ("WRITE_BLK_MISALIGN    : %d\n",         p->write_blk_misalign);
    printf ("READ_BLK_MISALIGN     : %d\n",         p->read_blk_misalign);
    printf ("DSR_IMP               : %d\n",         p->dsr_imp);
    printf ("VDD_R_CURR_MIN        : %d (%d ua)\n", p->vdd_r_curr_min, vdd_values [p->vdd_r_curr_min]);
    printf ("VDD_R_CURR_MAX        : %d (%d ua)\n", p->vdd_r_curr_max, vdd_values [p->vdd_r_curr_max]);
    printf ("VDD_W_CURR_MIN        : %d (%d ua)\n", p->vdd_w_curr_min, vdd_values [p->vdd_w_curr_min]);
    printf ("VDD_W_CURR_MAX        : %d (%d ua)\n", p->vdd_w_curr_max, vdd_values [p->vdd_w_curr_max]);
    printf ("ERASE_GRP_SIZE        : %d\n",         ((p->erase_grp_size_hi << 2) | p->erase_grp_size_lo) + 1);
    printf ("ERASE_GRP_MULT        : %d\n",         p->erase_grp_mult + 1);
    printf ("WP_GRP_SIZE           : %d\n",         p->wp_grp_size + 1);
    printf ("WP_GRP_ENABLE         : %d\n",         p->wp_grp_enable);
    printf ("DEFAULT_ECC           : %d\n",         p->default_ecc);
    printf ("R2W_FACTOR            : %d (%d:1)\n",  p->r2w_factor, 1 << p->r2w_factor);
    printf ("COPY                  : %d\n",         p->copy);
    printf ("PERM_WRITE_PROTECT    : %d\n",         p->perm_write_protect);
    printf ("TMP_WRITE_PROTECT     : %d\n",         p->tmp_write_protect);
    printf ("ECC                   : %d\n",         p->ecc);
    printf ("C_SIZE                : %d\n",         (p->c_size_hi << 10) | (p->c_size_mid << 2) | p->c_size_lo);
    printf ("C_SIZE_MULT           : %d\n",         (p->c_size_mult_hi << 1) | p->c_size_mult_lo);
    printf ("MEDIA_SIZE            : %u\n",         (U32) (((p->c_size_hi << 10) | (p->c_size_mid << 2) | p->c_size_lo) + 1) *
                                                    (U32) (4 << ((p->c_size_mult_hi << 1) | p->c_size_mult_lo)) *
                                                    (U32) (1 << (p->read_bl_len)));
  }
  else
    printf ("CSD read error\n");
#else
  printf ("Not implemented\n");
#endif
}
