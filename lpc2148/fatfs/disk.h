//
//  $Id: disk.h 50 2008-10-05 03:21:54Z jcw $
//  $Revision: 50 $
//  $Author: jcw $
//  $Date: 2008-10-04 23:21:54 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fatfs/disk.h $
//

/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.04a   (C)ChaN, 2007
/-----------------------------------------------------------------------*/

#ifndef _DISK_H_
#define _DISK_H_

//
//  Results of Disk Functions 
//
typedef enum 
{
	DRESULT_OK = 0,		/* 0: Successful */
	DRESULT_ERROR,		/* 1: R/W Error */
	DRESULT_WRPRT,		/* 2: Write Protected */
	DRESULT_NOTRDY,		/* 3: Not Ready */
	DRESULT_PARERR		/* 4: Invalid Parameter */
} 
DRESULT;

typedef enum
{
  DSC_COMMANDPASS = 0, 
  DSC_NOTREADY, 
  DSC_NOTPRESENT,
  DSC_PARAMETERSERROR, 
  DSC_MISCOMPAREERROR, 
  DSC_DISKCHANGED,
  DSC_UNKNOWNERROR,
} 
diskStatusCode_e;

//
//  Status of Disk Functions 
//
typedef U8 DSTATUS;

//
//  Disk Status Bits (DSTATUS)
//
#define DSTATUS_NOINIT		0x01	/* Drive not initialized */
#define DSTATUS_NODISK		0x02	/* No media in the drive */
#define DSTATUS_PROTECT		0x04	/* Write protected */

//
//  Command code for disk_ioctl() 
//
#define IOCTL_GET_SECTOR_COUNT	1
#define IOCTL_GET_SECTOR_SIZE		2
#define IOCTL_CTRL_SYNC			    3
#define IOCTL_CTRL_POWER			  4
#define IOCTL_CTRL_LOCK			    5
#define IOCTL_CTRL_EJECT			  6
#define IOCTL_MMC_GET_CSD			 10
#define IOCTL_MMC_GET_CID			 11
#define IOCTL_MMC_GET_OCR			 12
#define IOCTL_ATA_GET_REV			 20
#define IOCTL_ATA_GET_MODEL		 21
#define IOCTL_ATA_GET_SN			 22

//
//
//
typedef struct
{
  diskStatusCode_e statusCode;
  BOOL writeProtect;
  BOOL mediaChanged;
} 
mediaStatus_t;

//
//
//
DSTATUS diskInitialize (U8);
DSTATUS diskShutdown (void);
DSTATUS diskStatus (U8);
DRESULT diskRead (U8, U8 *, U32, U8);
#if _FS_READONLY == 0
DRESULT diskWrite (U8, const U8 *, U32, U8);
#endif
DRESULT diskIoctl (U8, U8, void *);
BOOL diskPresent (void);
const char *diskErrorText (DRESULT d);
void diskErrorTextPrint (DRESULT d);

#endif
