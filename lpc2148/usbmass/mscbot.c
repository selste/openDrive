//
//  $Id: mscbot.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbmass/mscbot.c $
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

#include "../usb/usbapi.h"
#include "mscdebug.h"
#include "mscbot.h"
#include "mscscsi.h"

//
// Command block wrapper structure
//
typedef struct 
{
  U32   dwCBWSignature;
  U32   dwCBWTag;
  U32   dwCBWDataTransferLength;
  U8    bmCBWFlags;
  U8    bCBWLun;
  U8    bCBWCBLength;
  U8    CBWCB[16];
} 
TCBW;

//
//  Command status wrapper structure 
//
typedef struct 
{
  U32   dwCSWSignature;
  U32   dwCSWTag;
  U32   dwCSWDataResidue;
  U8    bmCSWStatus;
} 
TCSW;

//
//  States of BOT state machine 
//
typedef enum 
{
  eCBW,
  eDataOut,
  eDataIn,
  eCSW,
  eStalled
} 
EBotState;

//
//
//
#define CBW_SIGNATURE 0x43425355    /**< magic word in CBW */
#define CSW_SIGNATURE 0x53425355    /**< magic word in CSW */

#define STATUS_PASSED     0x00    /**< successful transfer */
#define STATUS_FAILED     0x01    /**< failed transfer */
#define STATUS_PHASE_ERR  0x02    /**< conflict between host and device */

static U32 dwTransferSize;  /**< total size of data transfer */
static U32 dwOffset;        /**< offset in current data transfer */
static TCBW CBW;
static TCSW CSW;
static EBotState  eState;
static U8 *pbData;

//
//  Prepares a CSW, to be sent on next bulk-IN interrupt
//    
//  @param [in] bStatus CSW status
//
static void mscbotSendCSW (U8 bStatus)
{
  int iResidue;
  
  iResidue = CBW.dwCBWDataTransferLength - dwTransferSize;
  
  CSW.dwCSWSignature = CSW_SIGNATURE;
  CSW.dwCSWTag = CBW.dwCBWTag;
  CSW.dwCSWDataResidue = MAX (iResidue, 0);
  CSW.bmCSWStatus = bStatus;

  DBG ("CSW: status=%x, residue=%d\n", bStatus, CSW.dwCSWDataResidue);

  eState = eCSW;
}

//
//  Checks if CBW is valid and meaningful
//    
//  @param [in] pCBW  Command block wrapper
//  @param [in] iLen  Length of CBW
//      
//  @return TRUE if valid and meaningful
//
static BOOL mscbotCheckCBW (TCBW *pCBW, int iLen)
{
  //
  // CBW valid?
  //
  if (iLen != 31) 
  {
    DBG ("Invalid length (%d)\n", iLen);
    return FALSE;
  }

  if (pCBW->dwCBWSignature != CBW_SIGNATURE) 
  {
    DBG ("Invalid signature %x\n", pCBW->dwCBWSignature);
    return FALSE;
  }

  //
  // CBW meaningful?
  //
  if (pCBW->bCBWLun != 0) 
  {
    DBG ("Invalid LUN %d\n", pCBW->bCBWLun);
    return FALSE;
  }

  if ((pCBW->bCBWCBLength < 1) || (pCBW->bCBWCBLength > 16)) 
  {
    DBG ("Invalid CB len %d\n", pCBW->bCBWCBLength);
    return FALSE;
  }

  return TRUE;
}

//
//  mscbotBOTStall
//    Local function to stall ongoing transfer
//    
//  Which endpoint to stall is determined by looking at the transfer
//  direction intended by the host.
//
static void mscbotBOTStall (void)
{
  if ((CBW.bmCBWFlags & 0x80) || (CBW.dwCBWDataTransferLength == 0)) 
    usbHardwareEndpointStall (MSC_BULK_IN_EP, TRUE);
  else 
    usbHardwareEndpointStall (MSC_BULK_OUT_EP, TRUE);
}

//
//  mscbotHandleDataIn
//    Handles data from device-to-host
//    
static void mscbotHandleDataIn (void)
{
  int iChunk;
  
  //
  //  Process data for host in SCSI layer
  //
  if ((pbData = mscscsiHandleData (CBW.CBWCB, CBW.bCBWCBLength, pbData, dwOffset)) == NULL)
  {
    mscbotBOTStall ();
    mscbotSendCSW (STATUS_FAILED);
    return;
  }

  //
  //  Send data to host?
  //
  if (dwOffset < dwTransferSize) 
  {
    iChunk = MIN (64, dwTransferSize - dwOffset);
    usbHardwareEndpointWrite (MSC_BULK_IN_EP, pbData, iChunk);
    dwOffset += iChunk;
  }
  
  //
  // are we done now?
  //
  if (dwOffset == dwTransferSize) 
  {
    if (dwOffset != CBW.dwCBWDataTransferLength) 
    {
      DBG ("stalling DIN");
      mscbotBOTStall ();
    }

    //
    //  Done
    //
    mscbotSendCSW (STATUS_PASSED);
  }
}

//
//  mscbotHandleDataOut
//    Handles data from host-to-device
//
static void mscbotHandleDataOut (void)
{
  int iChunk;

  if (dwOffset < dwTransferSize) 
  {
    iChunk = usbHardwareEndpointRead (MSC_BULK_OUT_EP, pbData, dwTransferSize - dwOffset);
    pbData = mscscsiHandleData (CBW.CBWCB, CBW.bCBWCBLength, pbData, dwOffset);

    if (pbData == NULL) 
    {
      mscbotBOTStall ();
      mscbotSendCSW (STATUS_FAILED);
      return;
    }

    dwOffset += iChunk;
  }

  //
  //  Are we done now?
  //
  if (dwOffset == dwTransferSize) 
  {
    if (dwOffset != CBW.dwCBWDataTransferLength) 
    {
      DBG ("stalling DOUT");
      mscbotBOTStall ();
    }

    mscbotSendCSW (STATUS_PASSED);
  }
}
    
//
//  Resets the BOT state machine
//
void mscbotReset (void)
{
  DBG ("BOT reset in state %d\n", eState);
  eState = eCBW;
  mscscsiReset ();
}

//
//   Handles the BOT bulk OUT endpoint
//     
//   @param [in] bEP     Endpoint number
//   @param [in] bEPStatus Endpoint status (indicates NAK, STALL, etc)
//     
int mscbotBulkOut (U8 bEP, U8 bEPStatus)
{
  unsigned int iLen, iChunk;
  BOOL fHostIn, fDevIn;

  //
  //  Ignore events on stalled EP
  //
  if (bEPStatus & EP_STATUS_STALLED)
    return 0;

  switch (eState) 
  {
    case eCBW:
      {
        iLen = usbHardwareEndpointRead (bEP, (U8 *)&CBW, sizeof(CBW));

        if (!mscbotCheckCBW(&CBW, iLen)) 
        {
          usbHardwareEndpointStall (MSC_BULK_IN_EP, TRUE);
          usbHardwareEndpointStall (MSC_BULK_OUT_EP, TRUE);
          eState = eStalled;
          break;
        }

        DBG ("CBW: len=%d, flags=%x, cmd=%x, cmdlen=%d\n", CBW.dwCBWDataTransferLength, CBW.bmCBWFlags, CBW.CBWCB[0], CBW.bCBWCBLength);

        dwOffset = 0;
        dwTransferSize = 0;
        fHostIn = ((CBW.bmCBWFlags & 0x80) != 0);

        //
        //  Verify request.  Unknown command if NULL
        //
        if ((pbData = mscscsiHandleCmd (CBW.CBWCB, CBW.bCBWCBLength, &iLen, &fDevIn)) == NULL)
        {
          mscbotBOTStall ();
          mscbotSendCSW (STATUS_FAILED);
          break;      
        }

        //
        //  Rule: if device and host disagree on direction, send CSW with status 2
        //
        if ((iLen > 0) && ((fHostIn && !fDevIn) || (!fHostIn && fDevIn))) 
        {
          DBG ("Host and device disagree on direction\n");
          mscbotBOTStall ();
          mscbotSendCSW (STATUS_PHASE_ERR);
          break;
        }

        //
        //  Rule: if D > H, send CSW with status 2
        //
        if (iLen > CBW.dwCBWDataTransferLength) 
        {
          DBG ("Negative residue\n");
          mscbotBOTStall ();
          mscbotSendCSW (STATUS_PHASE_ERR);
          break;
        }

        if (((dwTransferSize = iLen) == 0) || fDevIn) 
        {
          //
          //  Data from device-to-host
          //
          eState = eDataIn;
          mscbotHandleDataIn ();
        }
        else 
        {
          //
          //  Data from host-to-device
          //
          eState = eDataOut;
        }
      }
      break;

    case eDataOut:
      {
        mscbotHandleDataOut ();
      }
      break;

    case eDataIn:
    case eCSW:
      {
        iChunk = usbHardwareEndpointRead (bEP, NULL, 0);
        DBG ("Phase error in state %d, %d bytes\n", eState, iChunk);
        eState = eCBW;
      }
      break;

    case eStalled:
      {
        usbHardwareEndpointStall (MSC_BULK_OUT_EP, TRUE);
      }
      break;

    default:
      {
        DBG ("Invalid state %d\n", eState);
        ASSERT (FALSE);
      }
      break;
  }

  return 0;
}

//
//   Handles the BOT bulk IN endpoint
//     
//   @param [in] bEP     Endpoint number
//   @param [in] bEPStatus Endpoint status (indicates NAK, STALL, etc)
//     
int mscbotBulkIn (U8 bEP __attribute__ ((unused)), U8 bEPStatus)
{
  if (bEPStatus & EP_STATUS_STALLED)
    return 0;

  switch (eState) 
  {
    //
    //  Ignore possibly old ACKs
    //
    case eCBW:
    case eDataOut:
      break;

    case eDataIn:
      mscbotHandleDataIn ();
      break;

    //
    //  Wait for an IN token, then send the CSW
    //
    case eCSW:
      usbHardwareEndpointWrite (MSC_BULK_IN_EP, (U8 *)&CSW, 13);
      eState = eCBW;
      break;

    //
    //  Keep stalling
    //
    case eStalled:
      usbHardwareEndpointStall (MSC_BULK_IN_EP, TRUE);
      break;

    default:
      DBG ("Invalid state %d\n", eState);
      ASSERT (FALSE);
      break;
  }

  return 0;
}
