#include "debug.h"

#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pwm_port.h"


void vPWMPort_Init(void) {

	LPC_SC->PCONP |= (1 << LPC_SC_PCONP_PCPWM1);					// Enable Power for PWM1
    LPC_SC->PCLKSEL0 |= (0b01 << LPC_SC_PCLKSEL0_PCLK_PWM1);		// Clock Prescaler (CCLK/1)
    
    LPC_PINCON->PINSEL4 &= ~((0b11 << LPC_PINCON_PINSEL4_P2_4));	// Reset PortPin 2.4
	LPC_PINCON->PINSEL4 |= (0b01 << LPC_PINCON_PINSEL4_P2_4);		// P2.4 => PWM1.5

	//LPC_PINCON->PINSEL4 &= ~((0b11 << LPC_PINCON_PINSEL4_P2_4));	// Reset PortMode 2.4
    //LPC_PINCON->PINMODE4 |= (0b11 << LPC_PINCON_PINMODE4_P2_4);		// PullDown Resistor

	// Prescale Register. The TC is incremented every PR+1 cycles of PCLK.
	LPC_PWM1->PR = 0;

	// PWM Timer Control Register
	LPC_PWM1->TCR = (1 << LPC_PWM1_TCR_CounterReset);	// Counter reset

	// PWM Count Control Register
	LPC_PWM1->CTCR = (0b00 << LPC_PWM1_CTCR_CTMode);	// Timer Mode: the TC is incremented when the Prescale Counter matches the Prescale Register.

	// PWM Match Control Register
	LPC_PWM1->MCR = (0 << LPC_PWM1_MCR_PWMMR5I)			// No interrupt
	              | (0 << LPC_PWM1_MCR_PWMMR5R)			// Reset on PWMMR5: the PWMTC will be reset if PWMMR5 matches it.
				  | (0 << LPC_PWM1_MCR_PWMMR5S);		// Do not stop

	// PWM Control Register
	LPC_PWM1->PCR = (0 << LPC_PWM1_PCR_PWMSEL5);		// use Single-Edge-Mode


	LPC_PWM1->MR0 = PWMPort_MR0;
    LPC_PWM1->MR5 = 0;

	LPC_PWM1->LER = (1 << LPC_PWM1_LER_MR0Latch)
	              | (1 << LPC_PWM1_LER_MR5Latch);





}

void vPWMPort_SetMR(uint32_t value) {
	
	LPC_PWM1->MR0 = PWMPort_MR0;
	LPC_PWM1->MR5 = value;

	LPC_PWM1->LER = (1 << LPC_PWM1_LER_MR0Latch)
	              | (1 << LPC_PWM1_LER_MR5Latch);
}

void vPWMPort_StartPWM(void) {

	LPC_PWM1->PCR = (1 << LPC_PWM1_PCR_PWMENA5);
	LPC_PWM1->TCR = (1 << LPC_PWM1_TCR_CounterEnable) | (1 << LPC_PWM1_TCR_PWMEnable);
	
}

void vPWMPort_StopPWM(void) {
	
	LPC_PWM1->PCR = 0;
	LPC_PWM1->TCR = 0;

}