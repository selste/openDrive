//
//  $Id: ssp.h 96 2008-10-10 15:29:59Z jcw $
//  $Revision: 96 $
//  $Author: jcw $
//  $Date: 2008-10-10 11:29:59 -0400 (Fri, 10 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/mmc/ssp.h $
//

#ifndef _SSP_H_
#define _SSP_H_

#include "sysdefs.h"

//
//  Hardware depends definitions
//
#define IDENTIFICATIONMODECLOCK   400000l
#define SYSTEMPERIPHERIALCLOCK  48000000l
#define SPI_DLY_1MSEC                1000

//
//
//
inline int sspHardwareCardPresent (void);
inline int sspHardwareWriteProtected (void);
void sspChipSelect (BOOL select);
inline BOOL sspPresent (void);
inline BOOL sspWriteProtect (void);
U32 sspSetClockFreq (U32 frequency);
void sspInit (void);
U8 sspTransferByte (U8 c);
U8 sspWaitReady (void);
void sspSendBlock (const U8 *pData, U32 size);
void sspReceiveBlock (U8 *pData, U32 size);
void sspDelay1ms (U32 delay);

#endif
