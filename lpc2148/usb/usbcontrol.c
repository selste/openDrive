//
//  $Id: usbcontrol.c 303 2008-11-07 03:48:15Z jcw $
//  $Revision: 303 $
//  $Author: jcw $
//  $Date: 2008-11-06 22:48:15 -0500 (Thu, 06 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbcontrol.c $
//

/** @file
  Control transfer handler.
  
  This module handles control transfers and is normally installed on the
  endpoint 0 callback.
  
  Control transfers can be of the following type:
  0 Standard;
  1 Class;
  2 Vendor;
  3 Reserved.

  A callback can be installed for each of these control transfers using
  USBRegisterRequestHandler.
  When an OUT request arrives, data is collected in the data store provided
  with the usbRegisterRequestHandler call. When the transfer is done, the
  callback is called.
  When an IN request arrives, the callback is called immediately to either
  put the control transfer data in the data store, or to get a pointer to
  control transfer data. The data is then packetised and sent to the host.
*/

#include "FreeRTOS.h"

#include "usbstruct.h"
#include "usbapi.h"

//
//
//
#define MAX_CONTROL_SIZE  128 /**< maximum total size of control transfer data */
#define MAX_REQ_HANDLERS  4   /**< standard, class, vendor, reserved */

static TSetupPacket Setup;    /**< setup packet */

static U8  *pbData;   /**< pointer to data buffer */
static int iResidue;  /**< remaining bytes in buffer */
static int iLen;      /**< total length of control transfer */

static TFnHandleRequest *apfnReqHandlers [4] = { NULL, NULL, NULL, NULL };  // Array of installed request handler callbacks
static U8 *apbDataStore [4] = { NULL, NULL, NULL, NULL };                   // Array of installed request data pointers */

//
//  Local function to handle a request by calling one of the installed
//  request handlers.
    
//  In case of data going from host to device, the data is at *ppbData.
//  In case of data going from device to host, the handler can either
//  choose to write its data at *ppbData or update the data pointer.
//
static BOOL _HandleRequest (TSetupPacket *pSetup, int *piLen, U8 **ppbData)
{
  TFnHandleRequest *pfnHandler;
  int iType;
  
  iType = REQTYPE_GET_TYPE (pSetup->bmRequestType);
  pfnHandler = apfnReqHandlers [iType];

  if (pfnHandler == NULL)
    return FALSE;

  return pfnHandler (pSetup, piLen, ppbData);
}

//
//  Local function to stall the control endpoint
//
static void usbStallControlPipe (U8 bEPStat __attribute__ ((unused)))
{
  usbHardwareEndpointStall (0x80, TRUE);
}

//
//  Sends next chunk of data (possibly 0 bytes) to host
//
static void usbDataIn (void)
{
  int iChunk;

  iChunk = MIN (MAX_PACKET_SIZE0, iResidue);
  usbHardwareEndpointWrite (0x80, pbData, iChunk);
  pbData += iChunk;
  iResidue -= iChunk;
}

//
//  Handles IN/OUT transfers on EP0
//
int usbHandleControlTransfer (U8 bEP, U8 bEPStat)
{
  int iChunk;
  int iType;

  //
  //  OUT transfer
  //
  if (bEP == 0x00) 
  {
    if (bEPStat & EP_STATUS_SETUP) 
    {
      //
      //  Setup packet, reset request message state machine
      //
      usbHardwareEndpointRead (0x00, (U8 *) &Setup, sizeof (Setup));

      //
      //  Defaults for data pointer and residue
      //
      iType = REQTYPE_GET_TYPE (Setup.bmRequestType);
      pbData = apbDataStore [iType];
      iResidue = Setup.wLength;
      iLen = Setup.wLength;

      if ((Setup.wLength == 0) || (REQTYPE_GET_DIR (Setup.bmRequestType) == REQTYPE_DIR_TO_HOST)) 
      {
        if (!_HandleRequest (&Setup, &iLen, &pbData)) 
        {
          usbStallControlPipe (bEPStat);
          return pdFALSE;
        }

        iResidue = MIN (iLen, Setup.wLength);
        usbDataIn ();
      }
    }
    else 
    {    
      if (iResidue > 0) 
      {
        iChunk = usbHardwareEndpointRead (0x00, pbData, iResidue);

        if (iChunk < 0) 
        {
          usbStallControlPipe (bEPStat);
          return pdFALSE;
        }

        pbData += iChunk;
        iResidue -= iChunk;

        if (iResidue == 0) 
        {
          iType = REQTYPE_GET_TYPE (Setup.bmRequestType);
          pbData = apbDataStore [iType];

          if (!_HandleRequest (&Setup, &iLen, &pbData)) 
          {
            usbStallControlPipe (bEPStat);
            return pdFALSE;
          }

          usbDataIn ();
        }
      }
      else 
        iChunk = usbHardwareEndpointRead (0x00, NULL, 0);
    }
  }
  else if (bEP == 0x80) 
    usbDataIn ();

  return pdFALSE;
}


//
//  Registers a callback for handling requests
//
void usbRegisterRequestHandler (int iType, TFnHandleRequest *pfnHandler, U8 *pbDataStore)
{
  apfnReqHandlers [iType] = pfnHandler;
  apbDataStore [iType] = pbDataStore;
}
