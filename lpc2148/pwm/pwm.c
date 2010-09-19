//
//  $Id: pwm.c 350 2009-01-12 01:25:46Z jcw $
//  $Revision: 350 $
//  $Author: jcw $
//  $Date: 2009-01-11 20:25:46 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/pwm/pwm.c $
//

#include "FreeRTOS.h"

#include "pwm.h"

//
//
//
static int pwmPulseWidth = 20000;
static int pwmPercentage = 50;

//
//
//
int pwmDutyCycle (int percentage)
{
  if ((percentage < 0) || (percentage > 100))
    return -1;

  PWM_MR5 = ((configCPU_CLOCK_HZ / pwmPulseWidth) * (pwmPercentage = percentage)) / 100;
  PWM_LER = PWM_LER_M5L;

  return 0;
}

//
//
//
int pwmPulseWidthInMicroseconds (int us)
{
  unsigned int count = (configCPU_CLOCK_HZ / 1000000) * us;

  if (count > PWM_MR0)
    return -1;

  PWM_MR5 = count;
  PWM_LER = PWM_LER_M5L;

  return 0;
}

//
//
//
int pwmFrequency (int frequency)
{
  if ((frequency < 1) || (frequency >= (int) configCPU_CLOCK_HZ))
    return -1;

  PWM_MR0 = (configCPU_CLOCK_HZ / (pwmPulseWidth = frequency));
  PWM_MR5 = ((configCPU_CLOCK_HZ / pwmPulseWidth) * pwmPercentage) / 100;
  PWM_LER = PWM_LER_M5L | PWM_LER_M0L;

  return 0;
}

//
//
//
void pwmInit (void)
{
  SCB_PCONP |= SCB_PCONP_PCPWM0;
  PCB_PINSEL1 = (PCB_PINSEL1 & ~PCB_PINSEL1_P021_MASK) | PCB_PINSEL1_P021_PWM5;

  PWM_TCR  = PWM_TCR_CR;
  PWM_PR   = 0;
  PWM_MR0  = (configCPU_CLOCK_HZ / pwmPulseWidth);
  PWM_MCR |= PWM_MCR_MR0R;
  PWM_PCR |= PWM_PCR_ENA5;
  PWM_TCR  = (PWM_TCR_CE | PWM_TCR_PWME);

  pwmDutyCycle (50);
}
