//
//  $Id: mscscsi.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbmass/mscscsi.c $
//

/*
  LPCUSB, an USB device driver for LPC microcontrollers 
  Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products
     derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FreeRTOS.h"

#include <string.h>

#include "../fatfs/disk.h"
#include "mscdebug.h"
#include "mscscsi.h"

//
//
//
#define BLOCKSIZE   512

//
//  SBC2 mandatory SCSI commands
//
#define SCSI_CMD_TEST_UNIT_READY  0x00
#define SCSI_CMD_REQUEST_SENSE    0x03
#define SCSI_CMD_FORMAT_UNIT      0x04
#define SCSI_CMD_READ_6           0x08  /* not implemented yet */
#define SCSI_CMD_INQUIRY          0x12
#define SCSI_CMD_SEND_DIAGNOSTIC  0x1D  /* not implemented yet */
#define SCSI_CMD_READ_CAPACITY_10 0x25
#define SCSI_CMD_READ_10          0x28
#define SCSI_CMD_REPORT_LUNS      0xa0  /* not implemented yet */

//
//  SBC2 optional SCSI commands
//
#define SCSI_CMD_WRITE_6          0x0a  /* not implemented yet */
#define SCSI_CMD_WRITE_10         0x2a
#define SCSI_CMD_VERIFY_10        0x2f  /* required for windows format */

//
//  Sense codes
//
#define WRITE_ERROR           0x030C00
#define READ_ERROR            0x031100
#define INVALID_CMD_OPCODE    0x052000
#define INVALID_FIELD_IN_CDB  0x052400

//
//  Sense code, which is set on error conditions
//  Hex: 00aabbcc, where aa=KEY, bb=ASC, cc=ASCQ
//
static U32 dwSense;  

static const U8   abInquiry [] = 
{
  0x00,   // PDT = direct-access device
  0x80,   // removeable medium bit = set
  0x05,   // version = complies to SPC3
  0x02,   // response data format = SPC3
  0x1F,   // additional length
  0x00,
  0x00,
  0x00,
  'L','P','C','U','S','B',' ',' ',  // vendor
  'M','a','s','s',' ','s','t','o',  // product
  'r','a','g','e',' ',' ',' ',' ',
  '0','.','1',' '                   // revision
};

//
//  Data for "request sense" command. The 0xff are filled in later
//
static const U8 abSense [] = { 0x70, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0a, 
                               0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
                               0x00, 0x00 };

//
//  Buffer for holding one block of disk data
//
static U8 abBlockBuf [512];

typedef struct 
{
  U8    bOperationCode;
  U8    abLBA [3];
  U8    bLength;
  U8    bControl;
} 
TCDB6;

//
//
//
void mscscsiReset (void)
{
  dwSense = 0;
}

//
//  SCSIHandleCmd
//    Verifies a SCSI CDB and indicates the direction and amount of data
//    that the device wants to transfer.
//    
//  If this call fails, a sense code is set in dwSense.
//
//  IN  pbCDB     - Command data block
//      iCDBLen   - Command data block len
//  OUT *piRspLen - Length of intended response data:
//      *pfDevIn  - TRUE if data is transferred from device-to-host
//  
//  Returns a pointer to the data exchange buffer if successful,
//  return NULL otherwise.
//
U8 *mscscsiHandleCmd (U8 *pbCDB, U8 iCDBLen, unsigned int *piRspLen, BOOL *pfDevIn)
{
  static const U8 aiCDBLen [] = {6, 10, 10, 0, 16, 12, 0, 0};
  int   i;
  TCDB6 *pCDB;
  U32   dwLen, dwLBA;
  U8    bGroupCode;

  pCDB = (TCDB6 *) pbCDB;

  // default direction is from device to host
  *pfDevIn = TRUE;

  // check CDB length
  bGroupCode = (pCDB->bOperationCode >> 5) & 0x7;

  if (iCDBLen < aiCDBLen [bGroupCode]) 
  {
    DBG ("Invalid CBD len (expected %d)!\n", aiCDBLen [bGroupCode]);
    return NULL;
  }

  switch (pCDB->bOperationCode) 
  {
    case SCSI_CMD_TEST_UNIT_READY :
      {
        DBG ("TEST UNIT READY\n");
        *piRspLen = 0;
      }
      break;

    case SCSI_CMD_REQUEST_SENSE :
      {
        DBG ("REQUEST SENSE (%06X)\n", dwSense);
        *piRspLen = MIN (18, pCDB->bLength);
      }
      break;

    case SCSI_CMD_FORMAT_UNIT :
      {
        DBG ("FORMAT UNIT %02X\n", pbCDB [1]);
        *piRspLen = 0;
      }
      break;

    case SCSI_CMD_INQUIRY :
      {
        DBG ("INQUIRY\n");
        *piRspLen = MIN (36, pCDB->bLength);
      }
      break;

    case SCSI_CMD_READ_CAPACITY_10 :
      {
        DBG ("READ CAPACITY\n");
        *piRspLen = 8;
      }
      break;

    case SCSI_CMD_READ_10 :
      {
        dwLBA = (pbCDB [2] << 24) | (pbCDB [3] << 16) | (pbCDB [4] << 8) | (pbCDB [5]);
        dwLen = (pbCDB [7] << 8) | pbCDB [8];
        DBG ("READ10, LBA=%d, len=%d\n", dwLBA, dwLen);
        *piRspLen = dwLen * BLOCKSIZE;
      }
      break;

    case SCSI_CMD_WRITE_10 :
      {
        dwLBA = (pbCDB [2] << 24) | (pbCDB [3] << 16) | (pbCDB [4] << 8) | (pbCDB [5]);
        dwLen = (pbCDB [7] << 8) | pbCDB [8];
        DBG ("WRITE10, LBA=%d, len=%d\n", dwLBA, dwLen);
        *piRspLen = dwLen * BLOCKSIZE;
        *pfDevIn = FALSE;
      }
      break;

    case SCSI_CMD_VERIFY_10 :
      {
        DBG ("VERIFY10\n");
        if ((pbCDB [1] & (1 << 1)) != 0) 
        {
          DBG ("BYTCHK not supported\n");
          return NULL;
        }
        *piRspLen = 0;
      }
      break;

    default :
      {
        DBG ("Unhandled SCSI: ");    
        for (i = 0; i < iCDBLen; i++)
          DBG (" %02X", pbCDB [i]);
        DBG ("\n");

        dwSense = INVALID_CMD_OPCODE;
        *piRspLen = 0;
      }
      return NULL;
  }

  return abBlockBuf;
}

//
//  SCSIHandleData
//    Handles a block of SCSI data.
//    
//  IN      pbCDB    . Command data block
//          iCDBLen  - Command data block len
//  IN/OUT  pbData   - Data buffer
//  IN      dwOffset - Offset in data
//  
//  Returns a pointer to the next data to be exchanged if successful,
//  returns NULL otherwise.
//
U8 *mscscsiHandleData (U8 *pbCDB, U8 iCDBLen __attribute__ ((unused)), U8 *pbData, U32 dwOffset)
{
  switch (((TCDB6 *) pbCDB)->bOperationCode) 
  {
    case SCSI_CMD_TEST_UNIT_READY :
      {
        if (dwSense != 0) 
          return NULL;
      }
      break;

    case SCSI_CMD_REQUEST_SENSE :
      {
        memcpy (pbData, abSense, 18);
        pbData [2]  = (dwSense >> 16) & 0xff;
        pbData [12] = (dwSense >> 8) & 0xff;
        pbData [13] = (dwSense >> 0) & 0xff;
        dwSense = 0;
      }
      break;

    case SCSI_CMD_FORMAT_UNIT :
      break;

    case SCSI_CMD_INQUIRY :
      {
        memcpy (pbData, abInquiry, sizeof (abInquiry));
      }
      break;

    case SCSI_CMD_READ_CAPACITY_10:
      {
        U32 dwBlockCount;

        if (diskIoctl (0, IOCTL_GET_SECTOR_COUNT, &dwBlockCount) != DRESULT_OK)
          return NULL;

        pbData [0] = (dwBlockCount >> 24) & 0xff;
        pbData [1] = (dwBlockCount >> 16) & 0xff;
        pbData [2] = (dwBlockCount >> 8) & 0xff;
        pbData [3] = (dwBlockCount >> 0) & 0xff;
        pbData [4] = (BLOCKSIZE >> 24) & 0xff;
        pbData [5] = (BLOCKSIZE >> 16) & 0xff;
        pbData [6] = (BLOCKSIZE >> 8) & 0xff;
        pbData [7] = (BLOCKSIZE >> 0) & 0xff;
      }
      break;

    case SCSI_CMD_READ_10 :
      {
        U32 dwLBA = (pbCDB [2] << 24) | (pbCDB [3] << 16) | (pbCDB [4] << 8) | (pbCDB [5]);
        U32 dwBufPos = (dwOffset & (BLOCKSIZE - 1));

        if (dwBufPos == 0) 
        {
          U32 dwBlockNr = dwLBA + (dwOffset / BLOCKSIZE);
          DBG ("R");
          if (diskRead (0, abBlockBuf, dwBlockNr, 1))
          {
            dwSense = READ_ERROR;
            DBG ("mscblockRead failed\n");
            return NULL;
          }
        }

        return abBlockBuf + dwBufPos;
      }

    case SCSI_CMD_WRITE_10:
      {
        U32 dwLBA = (pbCDB [2] << 24) | (pbCDB [3] << 16) | (pbCDB [4] << 8) | (pbCDB [5]);
        U32 dwBufPos = (dwOffset + 64) & (BLOCKSIZE - 1);

        if (dwBufPos == 0)
        {
          U32 dwBlockNr = dwLBA + (dwOffset / BLOCKSIZE);
          DBG ("W");
          if (diskWrite (0, abBlockBuf, dwBlockNr, 1))
          {
            dwSense = WRITE_ERROR;
            DBG ("mscblockWrite failed\n");
            return NULL;
          }
        }

        return abBlockBuf + dwBufPos;
      }

    case SCSI_CMD_VERIFY_10 :
      break;

    default :
      {
        dwSense = INVALID_CMD_OPCODE;
      }
      return NULL;
  }

  return abBlockBuf;
}
