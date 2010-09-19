//
//  $Id: lcd.h 283 2008-11-06 01:22:02Z jcw $
//  $Revision: 283 $
//  $Author: jcw $
//  $Date: 2008-11-05 20:22:02 -0500 (Wed, 05 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/lcd/lcd.h $
//

#ifndef _LCD_H_
#define _LCD_H_

#include "FreeRTOS.h"

//
//
//
void lcdWriteChar (char c);
void lcdWriteText (const char *s);
void lcdCursorXY (int x, int y);
void lcdInit (void);
signed portBASE_TYPE lcdTaskStart (void);
signed portBASE_TYPE lcdTaskStop (void);

#endif
