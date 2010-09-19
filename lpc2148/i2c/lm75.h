//
//  $Id: lm75.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/lm75.h $
//

#ifndef _LM75_H_
#define _LM75_H_

//
//
//
#define LM75_ADDRESS (0x90)

#define LM75_REGISTER_TEMPERATURE   (0x00)
#define LM75_REGISTER_CONFIGURATION (0x01)
#define LM75_REGISTER_THYST         (0x02)
#define LM75_REGISTER_TOS           (0x03)

//
//
//
int  lm75Init (void);
void lm75SetMode (int mode);
void lm75SetAddress (U8 address);
int  lm75ReRead (int *value);
int  lm75TemperatureRead (int *temp);
int  lm75ConfigRead (int *configValue);
int  lm75ConfigWrite (int configValue);
int  lm75THYSTRead (int *thystValue);
int  lm75THYSTWrite (int thystValue);
int  lm75TOSTRead (int *thystValue);
int  lm75TOSWrite (int thystValue);

#endif
