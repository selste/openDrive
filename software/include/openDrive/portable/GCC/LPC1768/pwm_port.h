
#ifndef __openDrive_PWMPORT_H
#define __openDrive_PWMPORT_H

#define PWMPort_MR0 3400

/*** Functions ***/
void vPWMPort_Init(void);
void vPWMPort_SetMR(uint32_t value);
void vPWMPort_StartPWM(void);
void vPWMPort_StopPWM(void);

#endif /* end __openDrive_PWMPORT_H */
