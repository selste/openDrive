//
//  $Id: disk.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fatfs/disk.c $
//

#include <stdio.h> 

#include "../mmc/mmc.h"
#include "disk.h"

#define S_MAX_SIZ 512
static volatile DSTATUS gDiskStatus = DSTATUS_NOINIT; 
static mediaStatus_t mediaStatus;

//
//
//
DSTATUS diskInitialize (U8 drv __attribute__ ((unused)))
{
  //
  //  Media Init
  //
  switch (mmcInitMedia ())
  {
    case MmcOk :
      {
        mediaStatus.statusCode = DSC_COMMANDPASS;
        mediaStatus.mediaChanged = TRUE;
        gDiskStatus = 0;
        if (mmcWriteProtect ())
        {
          mediaStatus.writeProtect = TRUE;
          gDiskStatus |= DSTATUS_PROTECT;
        }
      }
      break;

    case MmcCardError :
    case MmcDataError :
      {
        mediaStatus.statusCode = DSC_NOTREADY;
        gDiskStatus = DSTATUS_NOINIT;
        if (mmcWriteProtect ())
        {
          mediaStatus.writeProtect = TRUE;
          gDiskStatus |= DSTATUS_PROTECT;
        }
      }
      break;

    default:
      {
        mediaStatus.statusCode = DSC_NOTPRESENT;
        gDiskStatus = DSTATUS_NODISK;
      }
      break;
  }

  return gDiskStatus;
}

//
//
//
DSTATUS diskShutdown (void)
{
  gDiskStatus |= DSTATUS_NOINIT;

  return gDiskStatus;
}

//
//
//
DSTATUS diskStatus (U8 drv __attribute__ ((unused)))
{
  return gDiskStatus;
}

//
//
//
DRESULT diskRead (U8 disk __attribute__ ((unused)), U8 *buff, U32 sector, U8 count)
{
  U32 res;

  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;
  if (!count) 
    return DRESULT_PARERR;

  res = mmcRead (buff, sector, count);

  if (res == MMC_OK)
    return DRESULT_OK;
  else
    return DRESULT_ERROR; 
}

//
//
//
#if _FS_READONLY == 0
DRESULT diskWrite (U8 disk __attribute__ ((unused)), const U8 *buff, U32 sector, U8 count)
{
  U32 res;

  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;
  if (gDiskStatus & DSTATUS_PROTECT) 
    return DRESULT_WRPRT;
  if (!count) 
    return DRESULT_PARERR;

  res = mmcWrite (buff, sector, count);

  if (res == MMC_OK)
    return DRESULT_OK;
  else
    return DRESULT_ERROR; 
}
#endif

//
//
//
DRESULT diskIoctl (U8 drv, U8 ctrl, void *buff)
{
  DRESULT res;
  U8 n;
  U8 csd [16];
  U16 csize;

  if (drv) 
    return DRESULT_PARERR;
  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;

  res = DRESULT_ERROR;

  switch (ctrl) 
  {
    case IOCTL_GET_SECTOR_COUNT :
      { 
        if (mmcReadCardInfo (csd, CMD9) == MmcOk)                
        {
          //
          //  SDC ver 2.00
          //
          if ((csd [0] >> 6) == 1) 
          { 
            csize = csd [9] + ((U16)csd [8] << 8) + 1;
            *(U32 *) buff = (U32) csize << 10;
          } 
          //
          //  MMC or SDC ver 1.XX
          //
          else 
          {            
            n = (csd [5] & 15) + ((csd [10] & 128) >> 7) + ((csd [9] & 3) << 1) + 2;
            csize = (csd [8] >> 6) + ((U16) csd [7] << 2) + ((U16) (csd [6] & 3) << 10) + 1;
            *(U32 *) buff = (U32) csize << (n - 9);
          }

          res = DRESULT_OK;
        }
      }
      break;

    case IOCTL_GET_SECTOR_SIZE :
      {
#if S_MAX_SIZ > 512
        if (mmcDskStatus.blockSize)
          *(U16 *) buff = mmcDskStatus.blockSize;
        else
        {
          DBG ("mmcDskStatus.blockSize not set!\n");
          *(U16 *) buff = S_MAX_SIZ;
        }
#else
        *(U16 *) buff = S_MAX_SIZ;
#endif
        res = DRESULT_OK;
      }
      break;

    case IOCTL_CTRL_SYNC :
      {
        if (mmcWaitReady ())
          res = DRESULT_OK;
      }
      break;

    case IOCTL_MMC_GET_CSD :
      {
        if (mmcReadCardInfo (buff, CMD9) == MmcOk)
          res = DRESULT_OK;
      }
      break;

    case IOCTL_MMC_GET_CID :
      {
        if (mmcReadCardInfo (buff, CMD10) == MmcOk)
          res = DRESULT_OK;
      }
      break;

# if 0
    case IOCTL_CTRL_POWER:
    case IOCTL_CTRL_LOCK:
    case IOCTL_CTRL_EJECT:
    case IOCTL_MMC_GET_OCR:     /* Receive OCR as an R3 resp (4 bytes) */
    case IOCTL_ATA_GET_REV:
    case IOCTL_ATA_GET_MODEL:
    case IOCTL_ATA_GET_SN:
#endif                        

    default:
      res = DRESULT_PARERR;
  }

  return res;
}

//
//
//
BOOL diskPresent (void)
{
  return mmcCardPresent ();
}

//
//
//
const char *diskErrorText (DRESULT d)
{
  unsigned int i;

  typedef struct errorStrings_s
  {
    DRESULT dresult;
    const char *string;
  }
  errorStrings_t;

  static const errorStrings_t errorStrings [] =
  {
    { DRESULT_OK,       "OK"           },
    { DRESULT_ERROR,    "R/W ERROR"    },
    { DRESULT_WRPRT,    "WP ERROR"     },
    { DRESULT_NOTRDY,   "NOT READY"    },
    { DRESULT_PARERR,   "INVALID PARM" },
  };

  for (i = 0; i < arrsizeof (errorStrings); i++)
    if (errorStrings [i].dresult == d)
      return errorStrings [d].string;

  return "(no err text)";
}

void diskErrorTextPrint (DRESULT d)
{
  printf ("rrc=%u %s\n", d, diskErrorText (d));
}
