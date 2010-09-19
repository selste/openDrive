//
//  $Id: usbapi.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbapi.h $
//

#include "usbstruct.h"    // for TSetupPacket

/*************************************************************************
  USB configuration
**************************************************************************/

#define MAX_PACKET_SIZE0  64    /**< maximum packet size for EP 0 */

/*************************************************************************
  USB hardware interface
**************************************************************************/

// endpoint status sent through callback
#define EP_STATUS_DATA    (1<<0)    /**< EP has data */
#define EP_STATUS_STALLED (1<<1)    /**< EP is stalled */
#define EP_STATUS_SETUP   (1<<2)    /**< EP received setup packet */
#define EP_STATUS_ERROR   (1<<3)    /**< EP data was overwritten by setup packet */
#define EP_STATUS_NACKED  (1<<4)    /**< EP sent NAK */

// device status sent through callback
#define DEV_STATUS_CONNECT    (1<<0)  /**< device just got connected */
#define DEV_STATUS_SUSPEND    (1<<2)  /**< device entered suspend state */
#define DEV_STATUS_RESET    (1<<4)  /**< device just got reset */

// interrupt bits for NACK events in usbHardwareNakIntEnable
// (these bits conveniently coincide with the LPC214x USB controller bit)
#define INACK_NONE  (0)         /**< no interrupts */
#define INACK_CI    (1<<1)      /**< interrupt on NACK for control in */
#define INACK_CO    (1<<2)      /**< interrupt on NACK for control out */
#define INACK_II    (1<<3)      /**< interrupt on NACK for interrupt in */
#define INACK_IO    (1<<4)      /**< interrupt on NACK for interrupt out */
#define INACK_BI    (1<<5)      /**< interrupt on NACK for bulk in */
#define INACK_BO    (1<<6)      /**< interrupt on NACK for bulk out */

BOOL usbHardwareInit (void);
void usbSetupInterruptHandler (void);
void usbHardwareNakIntEnable (U8 bIntBits);
void usbHardwareConnect (BOOL fConnect);
void usbHardwareSetAddress (U8 bAddr);
void usbHardwareConfigDevice (BOOL fConfigured);

//
//  Endpoint operations
//
void usbHardwareEndpointConfig    (U8 bEP, U16 wMaxPacketSize);
int  usbHardwareEndpointRead      (U8 bEP, U8 *pbBuf, int iMaxLen);
int  usbHardwareEndpointWrite     (U8 bEP, U8 *pbBuf, int iLen);
void usbHardwareEndpointStall     (U8 bEP, BOOL fStall);
BOOL usbHardwareEndpointIsStalled (U8 bEP);

//
//  Endpoint interrupt handler callback
//
typedef int (TFnEPIntHandler) (U8 bEP, U8 bEPStatus);
void usbHardwareRegisterEPIntHandler  (U8 bEP, TFnEPIntHandler *pfnHandler);

//
//  Device status handler callback
//
typedef void (TFnDevIntHandler)  (U8 bDevStatus);
void usbHardwareRegisterDevIntHandler (TFnDevIntHandler *pfnHandler);

//
//  Frame event handler callback
//
typedef void (TFnFrameHandler)(U16 wFrame);
void usbHardwareRegisterFrameHandler(TFnFrameHandler *pfnHandler);


/*************************************************************************
  USB application interface
**************************************************************************/

// initialise the complete stack, including HW
BOOL usbRegisterHandlers (void);

/** Request handler callback (standard, vendor, class) */
typedef BOOL (TFnHandleRequest)(TSetupPacket *pSetup, int *piLen, U8 **ppbData);
void usbRegisterRequestHandler (int iType, TFnHandleRequest *pfnHandler, U8 *pbDataStore);
void usbRegisterCustomReqHandler (TFnHandleRequest *pfnHandler);

/** Descriptor handler callback */
typedef BOOL (TFnGetDescriptor)(U16 wTypeIndex, U16 wLangID, int *piLen, U8 **ppbData);

/** Default standard request handler */
BOOL usbHandleStandardRequest (TSetupPacket *pSetup, int *piLen, U8 **ppbData);

/** Default EP0 handler */
int usbHandleControlTransfer (U8 bEP, U8 bEPStat);

/** Descriptor handling */
void usbRegisterDescriptors (const U8 *pabDescriptors);
BOOL usbGetDescriptor (U16 wTypeIndex, U16 wLangID, int *piLen, U8 **ppbData);
