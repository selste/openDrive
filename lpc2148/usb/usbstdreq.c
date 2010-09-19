//
//  $Id: usbstdreq.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbstdreq.c $
//

/** @file
  Standard request handler.
  
  This modules handles the 'chapter 9' processing, specifically the
  standard device requests in table 9-3 from the universal serial bus
  specification revision 2.0
  
  Specific types of devices may specify additional requests (for example
  HID devices add a GET_DESCRIPTOR request for interfaces), but they
  will not be part of this module.

  @todo some requests have to return a request error if device not configured:
  @todo GET_INTERFACE, GET_STATUS, SET_INTERFACE, SYNCH_FRAME
  @todo this applies to the following if endpoint != 0:
  @todo SET_FEATURE, GET_FEATURE 
*/

#include "usbstruct.h"
#include "usbapi.h"

#define MAX_DESC_HANDLERS 4   /**< device, interface, endpoint, other */


//
//  General descriptor field offsets 
//
#define DESC_bLength          0 /**< length offset */
#define DESC_bDescriptorType  1 /**< descriptor type offset */  

//
//  Config descriptor field offsets 
//
#define CONF_DESC_wTotalLength        2 /**< total length offset */
#define CONF_DESC_bConfigurationValue 5 /**< configuration value offset */  
#define CONF_DESC_bmAttributes        7 /**< configuration characteristics */

//
//  Interface descriptor field offsets 
//
#define INTF_DESC_bAlternateSetting   3 /**< alternate setting offset */

//
//  Endpoint descriptor field offsets 
//
#define ENDP_DESC_bEndpointAddress    2 /**< endpoint address offset */
#define ENDP_DESC_wMaxPacketSize      4 /**< maximum packet size offset */


static U8 bConfiguration = 0;                       /** Currently selected configuration */
static TFnHandleRequest *pfnHandleCustomReq = NULL; /** Installed custom request handler */
static const U8 *pabDescrip = NULL;                 /** Pointer to registered descriptors */


//
//  Registers a pointer to a descriptor block containing all descriptors for the device.
//
void usbRegisterDescriptors (const U8 *pabDescriptors)
{
  pabDescrip = pabDescriptors;
}

//
//  Parses the list of installed USB descriptors and attempts to find the specified USB descriptor.
//
BOOL usbGetDescriptor (U16 wTypeIndex, U16 wLangID __attribute__ ((unused)), int *piLen, U8 **ppbData)
{
  U8  bType, bIndex;
  U8  *pab;
  int iCurIndex;

  bType = GET_DESC_TYPE (wTypeIndex);
  bIndex = GET_DESC_INDEX (wTypeIndex);

  pab = (U8 *)pabDescrip;
  iCurIndex = 0;

  while (pab [DESC_bLength] != 0) 
  {
    if (pab [DESC_bDescriptorType] == bType) 
    {
      if (iCurIndex == bIndex) 
      {
        *ppbData = pab;

        if (bType == DESC_CONFIGURATION) 
          *piLen =  (pab [CONF_DESC_wTotalLength]) | (pab [CONF_DESC_wTotalLength + 1] << 8);
        else 
          *piLen = pab [DESC_bLength];

        return TRUE;
      }

      iCurIndex++;
    }

    pab += pab [DESC_bLength];
  }

  return FALSE;
}

//
//  Configures the device according to the specified configuration index and
//  alternate setting by parsing the installed USB descriptor list.
//  A configuration index of 0 unconfigures the device.
//
static BOOL usbSetConfiguration (U8 bConfigIndex, U8 bAltSetting)
{
  U8  *pab;
  U8  bCurConfig, bCurAltSetting;
  U8  bEP;
  U16 wMaxPktSize;

  if (bConfigIndex == 0) 
    usbHardwareConfigDevice(FALSE);
  else 
  {
    pab = (U8 *) pabDescrip;
    bCurConfig = 0xFF;
    bCurAltSetting = 0xFF;

    while (pab [DESC_bLength] != 0) 
    {
      switch (pab [DESC_bDescriptorType]) 
      {
        case DESC_CONFIGURATION :
          {
            bCurConfig = pab [CONF_DESC_bConfigurationValue];
          }
          break;

        case DESC_INTERFACE :
          {
            bCurAltSetting = pab [INTF_DESC_bAlternateSetting];
          }
          break;

        case DESC_ENDPOINT :
          {
            if ((bCurConfig == bConfigIndex) && (bCurAltSetting == bAltSetting)) 
            {
              bEP = pab [ENDP_DESC_bEndpointAddress];
              wMaxPktSize = (pab [ENDP_DESC_wMaxPacketSize]) | (pab [ENDP_DESC_wMaxPacketSize + 1] << 8);
              usbHardwareEndpointConfig (bEP, wMaxPktSize);
            }
          }
          break;

        default :
          break;
      }

      pab += pab [DESC_bLength];
    }

    usbHardwareConfigDevice (TRUE);
  }

  return TRUE;
}

//
//  Local function to handle a standard device request
//
static BOOL usbHandleStdDeviceReq (TSetupPacket *pSetup, int *piLen, U8 **ppbData)
{
  U8  *pbData = *ppbData;

  switch (pSetup->bRequest) 
  {
    case REQ_GET_STATUS :
      {
        pbData [0] = 0;
        pbData [1] = 0;
        *piLen = 2;
      }
      break;

    case REQ_SET_ADDRESS :
      {
        usbHardwareSetAddress (pSetup->wValue);
      }
      break;

    case REQ_GET_DESCRIPTOR :
      return usbGetDescriptor (pSetup->wValue, pSetup->wIndex, piLen, ppbData);

    case REQ_GET_CONFIGURATION :
      {
        pbData [0] = bConfiguration;
        *piLen = 1;
      }
      break;

    case REQ_SET_CONFIGURATION :
      {
        if (!usbSetConfiguration(pSetup->wValue & 0xFF, 0))
          return FALSE;

        bConfiguration = pSetup->wValue & 0xFF; 
      }
      break;

    case REQ_CLEAR_FEATURE :
    case REQ_SET_FEATURE :
      {
        if (pSetup->wValue == FEA_REMOTE_WAKEUP) {
        }
        if (pSetup->wValue == FEA_TEST_MODE) {
        }
      }
      return FALSE;

    case REQ_SET_DESCRIPTOR :
      return FALSE;

    default :
      return FALSE;
  }

  return TRUE;
}

//
//  Local function to handle a standard interface request
//
static BOOL usbHandleStdInterfaceReq (TSetupPacket  *pSetup, int *piLen, U8 **ppbData)
{
  U8  *pbData = *ppbData;

  switch (pSetup->bRequest) 
  {
    case REQ_GET_STATUS :
      {
        pbData [0] = 0;
        pbData [1] = 0;
        *piLen = 2;
      }
      break;

    case REQ_CLEAR_FEATURE :
    case REQ_SET_FEATURE :
      return FALSE;

    case REQ_GET_INTERFACE :
      {
        pbData [0] = 0;
        *piLen = 1;
      }
      break;

    case REQ_SET_INTERFACE :
      {
        if (pSetup->wValue != 0)
          return FALSE;

        *piLen = 0;
      }
      break;

    default :
      return FALSE;
  }

  return TRUE;
}

//
//  Local function to handle a standard endpoint request
// 
static BOOL usbHandleStdEndPointReq (TSetupPacket *pSetup, int *piLen, U8 **ppbData)
{
  U8  *pbData = *ppbData;

  switch (pSetup->bRequest) 
  {
    case REQ_GET_STATUS :
      {
        pbData [0] = usbHardwareEndpointIsStalled (pSetup->wIndex) ? 1 : 0;
        pbData [1] = 0;
        *piLen = 2;
      }
      break;

    case REQ_CLEAR_FEATURE :
      {
        if (pSetup->wValue == FEA_ENDPOINT_HALT) 
        {
          usbHardwareEndpointStall(pSetup->wIndex, FALSE);
          break;
        }
      }
      return FALSE;

    case REQ_SET_FEATURE :
      {
        if (pSetup->wValue == FEA_ENDPOINT_HALT) 
        {
          usbHardwareEndpointStall(pSetup->wIndex, TRUE);
          break;
        }
      }
      return FALSE;

    case REQ_SYNCH_FRAME :
      return FALSE;

    default :
      return FALSE;
  }

  return TRUE;
}


//
//  Default handler for standard ('chapter 9') requests
//  If a custom request handler was installed, this handler is called first.
//
BOOL usbHandleStandardRequest (TSetupPacket  *pSetup, int *piLen, U8 **ppbData)
{
  if ((pfnHandleCustomReq != NULL) && pfnHandleCustomReq (pSetup, piLen, ppbData))
    return TRUE;

  switch (REQTYPE_GET_RECIP(pSetup->bmRequestType)) 
  {
    case REQTYPE_RECIP_DEVICE :    return usbHandleStdDeviceReq (pSetup, piLen, ppbData);
    case REQTYPE_RECIP_INTERFACE : return usbHandleStdInterfaceReq (pSetup, piLen, ppbData);
    case REQTYPE_RECIP_ENDPOINT :  return usbHandleStdEndPointReq (pSetup, piLen, ppbData);
    default :                      return FALSE;
  }
}

//
//  Registers a callback for custom device requests
// 
//  In usbHandleStandardRequest, the custom request handler gets a first
//  chance at handling the request before it is handed over to the 'chapter 9'
//  request handler.
// 
//  This can be used for example in HID devices, where a REQ_GET_DESCRIPTOR
//  request is sent to an interface, which is not covered by the 'chapter 9'
//  specification.
//    
void usbRegisterCustomReqHandler (TFnHandleRequest *pfnHandler)
{
  pfnHandleCustomReq = pfnHandler;
}

