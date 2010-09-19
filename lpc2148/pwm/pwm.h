//
//  $Id: pwm.h 347 2008-12-04 15:27:16Z jcw $
//  $Revision: 347 $
//  $Author: jcw $
//  $Date: 2008-12-04 10:27:16 -0500 (Thu, 04 Dec 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/pwm/pwm.h $
//

#ifndef _PWM_H_
#define _PWM_H_

int pwmDutyCycle (int percentage);
int pwmPulseWidthInMicroseconds (int us);
int pwmFrequency (int frequency);
void pwmInit (void);

#endif
