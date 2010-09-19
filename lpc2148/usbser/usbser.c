//
//  $Id: usbser.c 325 2008-11-09 04:58:13Z jcw $
//  $Revision: 325 $
//  $Author: jcw $
//  $Date: 2008-11-08 23:58:13 -0500 (Sat, 08 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbser/usbser.c $
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../usb/usbapi.h"
#include "usbser.h"

//
//
//
#define INT_IN_EP     0x81
#define BULK_OUT_EP   0x05
#define BULK_IN_EP    0x82

#define MAX_PACKET_SIZE 64

#define LE_WORD(x)    ((x)&0xFF),((x)>>8)

// CDC definitions
#define CS_INTERFACE      0x24
#define CS_ENDPOINT       0x25

#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

#define CONTROLLINE_DTR 0x01
#define CONTROLLINE_RTS 0x02

//
//  Data structure for GET_LINE_CODING / SET_LINE_CODING class requests
//
typedef struct 
{
  U32   dwDTERate;
  U8    bCharFormat;
  U8    bParityType;
  U8    bDataBits;
} 
TLineCoding;

static TLineCoding LineCoding = {115200, 0, 0, 8};
static U16 controlLines = 0;
static U8 abBulkBuf [64];
static U8 abClassReqData [8];
static xQueueHandle xRXQueue = NULL;
static xQueueHandle xTXQueue = NULL;
static int usbSerialInitialized = FALSE;

//
//
//
static const U8 abDescriptors [] = 
{
  //
  //  Device descriptor
  //
  0x12,
  DESC_DEVICE,
  LE_WORD(0x0101),      // bcdUSB
  0x02,                 // bDeviceClass
  0x00,                 // bDeviceSubClass
  0x00,                 // bDeviceProtocol
  MAX_PACKET_SIZE0,     // bMaxPacketSize
  LE_WORD(0xffff),      // idVendor
  LE_WORD(0x0005),      // idProduct
  LE_WORD(0x0100),      // bcdDevice
  0x01,                 // iManufacturer
  0x02,                 // iProduct
  0x03,                 // iSerialNumber
  0x01,                 // bNumConfigurations

  //
  //  Configuration descriptor
  //
  0x09,
  DESC_CONFIGURATION,
  LE_WORD(67),          // wTotalLength
  0x02,                 // bNumInterfaces
  0x01,                 // bConfigurationValue
  0x00,                 // iConfiguration
  0xc0,                 // bmAttributes
  0x32,                 // bMaxPower

  //
  //  Control class interface
  //
  0x09,
  DESC_INTERFACE,
  0x00,                 // bInterfaceNumber
  0x00,                 // bAlternateSetting
  0x01,                 // bNumEndPoints
  0x02,                 // bInterfaceClass
  0x02,                 // bInterfaceSubClass
  0x01,                 // bInterfaceProtocol, linux requires value of 1 for the cdc_acm module
  0x00,                 // iInterface

  //
  //  Header functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x00,
  LE_WORD(0x0110),

  //
  //  Call management functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x01,
  0x01,                 // bmCapabilities = device handles call management
  0x01,                 // bDataInterface

  //
  //  ACM functional descriptor
  //
  0x04,
  CS_INTERFACE,
  0x02,
  0x02,                 // bmCapabilities

  //
  //  Union functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x06,
  0x00,                 // bMasterInterface
  0x01,                 // bSlaveInterface0

  //
  //  Notification EP
  //
  0x07,
  DESC_ENDPOINT,
  INT_IN_EP,            // bEndpointAddress
  0x03,                 // bmAttributes = intr
  LE_WORD(8),           // wMaxPacketSize
  0x0a,                 // bInterval

  //
  //  Data class interface descriptor
  //
  0x09,
  DESC_INTERFACE,
  0x01,                 // bInterfaceNumber
  0x00,                 // bAlternateSetting
  0x02,                 // bNumEndPoints
  0x0a,                 // bInterfaceClass = data
  0x00,                 // bInterfaceSubClass
  0x00,                 // bInterfaceProtocol
  0x00,                 // iInterface

  //
  //  Data EP OUT
  //
  0x07,
  DESC_ENDPOINT,
  BULK_OUT_EP,              // bEndpointAddress
  0x02,                     // bmAttributes = bulk
  LE_WORD(MAX_PACKET_SIZE), // wMaxPacketSize
  0x00,                     // bInterval

  //
  //  Data EP in
  //
  0x07,
  DESC_ENDPOINT,
  BULK_IN_EP,               // bEndpointAddress
  0x02,                     // bmAttributes = bulk
  LE_WORD(MAX_PACKET_SIZE), // wMaxPacketSize
  0x00,                     // bInterval

  //
  //  String descriptors
  //
  0x04,
  DESC_STRING,
  LE_WORD(0x0409),

  0x0E,
  DESC_STRING,
  'L', 0, 'P', 0, 'C', 0, 'U', 0, 'S', 0, 'B', 0,

  0x14,
  DESC_STRING,
  'U', 0, 'S', 0, 'B', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0,

  0x12,
  DESC_STRING,
  'D', 0, 'E', 0, 'A', 0, 'D', 0, 'C', 0, '0', 0, 'D', 0, 'E', 0,

  //
  //  Terminating zero
  //
  0
};

//
//
//
static void usbHardwareFrameIntHandler (U16 wFrame __attribute__ ((unused)))
{
  usbHardwareNakIntEnable (INACK_BI);
}

//
//  Local function to handle incoming bulk data.  This needs to copy the incoming data to the VCOM RX queue.
//
static int usbserBulkOut (U8 bEP, U8 bEPStatus __attribute__ ((unused)))
{
  int i;
  int iLen;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

  iLen = usbHardwareEndpointRead (bEP, abBulkBuf, sizeof (abBulkBuf));

  for (i = 0; i < iLen; i++) 
    xQueueSendFromISR (xRXQueue, &abBulkBuf [i], &higherPriorityTaskWoken);

  return higherPriorityTaskWoken;
}

//
//  Local function to handle outgoing bulk data.  This needs to copy any user data in the VCOM TX queue to the bulk endpoint.
//
static int usbserBulkIn (U8 bEP, U8 bEPStatus __attribute__ ((unused)))
{
  int i;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;
  
  for (i = 0; i < MAX_PACKET_SIZE; i++)
    if (xQueueReceiveFromISR (xTXQueue, &abBulkBuf [i], &higherPriorityTaskWoken) != pdPASS)
      break;

  if (i == 0)
    usbHardwareNakIntEnable (INACK_NONE);
  else
    usbHardwareEndpointWrite (bEP, abBulkBuf, i);

  return higherPriorityTaskWoken;
}

//
//  Local function to handle the USB-CDC class requests
//
static BOOL usbserHandleClassRequest (TSetupPacket *pSetup, int *piLen, U8 **ppbData)
{
  switch (pSetup->bRequest) 
  {
    case SET_LINE_CODING:
      {
        memcpy ((U8 *) &LineCoding, *ppbData, 7);
        *piLen = 7;
      }
      break;

    case GET_LINE_CODING:
      {
        *ppbData = (U8 *) &LineCoding;
        *piLen = 7;
      }
      break;

    case SET_CONTROL_LINE_STATE:
      {
        controlLines = pSetup->wValue;
      }
      break;

    default:
      return FALSE;
  }

  return TRUE;
}

//
//  Writes one character to VCOM port
//
signed portBASE_TYPE usbserPutChar (signed portCHAR cOutChar, portTickType xBlockTime)
{
  if (!usbSerialInitialized)
    return pdFALSE;
  if (!(controlLines & CONTROLLINE_DTR))
    return pdTRUE;

  return xQueueSend (xTXQueue, &cOutChar, xBlockTime);
}

signed portBASE_TYPE usbserPutString (const signed portCHAR * const pcString, portTickType xBlockTime)
{
  signed portCHAR *pxNext;

  if (!usbSerialInitialized)
    return pdFALSE;
  if (!(controlLines & CONTROLLINE_DTR))
    return pdTRUE;

  pxNext = (signed portCHAR *) pcString;

  while (*pxNext)
  {
    if (!usbserPutChar (*pxNext, xBlockTime))
      return pdTRUE;

    pxNext++;
  }

  return pdTRUE;
}

//
//  Reads one character from VCOM port
//
signed portBASE_TYPE usbserGetChar (signed portCHAR *pcRxedChar, portTickType xBlockTime)
{
  if (!usbSerialInitialized)
    return pdFALSE;

  return xQueueReceive (xRXQueue, pcRxedChar, xBlockTime) ? pdTRUE : pdFALSE;
}

void usbserGetRxQueue (xQueueHandle *qh)
{
  *qh = xRXQueue;
}

//
//  Default reset handler gets overridden so we can clear line state on
//  disconnect (this is only really useful when the board is not powered
//  from the USB connection, because if it is, power is going away when
//  the cable is unplugged...)
//
static void usbHandleReset (U8 bDevStatus __attribute__ ((unused)))
{
  controlLines = 0;
}

//
//
//
void usbserInit (void)
{
  portENTER_CRITICAL ();
  usbRegisterHandlers ();
  usbHardwareRegisterDevIntHandler (usbHandleReset);
  usbRegisterDescriptors (abDescriptors);
  usbRegisterRequestHandler (REQTYPE_TYPE_CLASS, usbserHandleClassRequest, abClassReqData);
  usbHardwareRegisterEPIntHandler (INT_IN_EP, NULL);
  usbHardwareRegisterEPIntHandler (BULK_IN_EP, usbserBulkIn);
  usbHardwareRegisterEPIntHandler (BULK_OUT_EP, usbserBulkOut);
  usbHardwareRegisterFrameHandler (usbHardwareFrameIntHandler);
  usbHardwareNakIntEnable (INACK_BI);
  usbSetupInterruptHandler ();
  xRXQueue = xQueueCreate (256, (unsigned portBASE_TYPE) sizeof (signed portCHAR));
  xTXQueue = xQueueCreate (256, (unsigned portBASE_TYPE) sizeof (signed portCHAR));
  usbSerialInitialized = TRUE;
  portEXIT_CRITICAL ();

  usbHardwareConnect (TRUE);
}
