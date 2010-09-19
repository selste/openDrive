//
//  $Id: fiq.h 323 2008-11-09 04:56:50Z jcw $
//  $Revision: 323 $
//  $Author: jcw $
//  $Date: 2008-11-08 23:56:50 -0500 (Sat, 08 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fiq/fiq.h $
//

#ifndef _FIQ_H_
#define _FIQ_H_

void fiqInit (void);
int fiqEnable (void);
int fiqDisable (void);
unsigned int fiqGetCount (void);
void fiqClearCount (void);
unsigned char *fiqFIQISRCopyToRAM (void);

#endif
