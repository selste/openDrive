#ifndef __openDrive_TMC428_H 
#define __openDrive_TMC428_H


#define TMC428_MotorRA 0
#define TMC428_MotorDEC 1

/*********************************************************/
/** Register-Bit Definitions                             */
/*********************************************************/
typedef struct {
	uint16_t VMax;
	uint16_t AMax;
	uint8_t PulseDiv;
	uint8_t RampDiv;
} TMC428_MotorConfig_t;


/*********************************************************/
/** Configure Hardwaredependend SPI-Functions            */
/*********************************************************/
#define TMC428_SPITRANSMIT xSSP1Port_Transmit
#define TMC428_SPISTART SSP1_SSEL_LOW
#define TMC428_SPISTOP SSP1_SSEL_HIGH


/*********************************************************/
/** Register-Adress Definitions                          */
/*********************************************************/
#define MOTOR0 0x00
#define MOTOR1 0x20
#define MOTOR2 0x40

#define IDX_XTARGET 0x00
#define IDX_XACTUAL 0x02
#define IDX_VMIN 0x04
#define IDX_VMAX 0x06
#define IDX_VTARGET 0x08
#define IDX_VACTUAL 0x0A
#define IDX_AMAX 0x0C
#define IDX_AACTUAL 0x0E
#define IDX_AGTAT_ALEAT 0x10
#define IDX_PMUL_PDIV 0x12
#define IDX_REFCONF_RM 0x14
#define IDX_IMASK_IFLAGS 0x16
#define IDX_PULSEDIV_RAMPDIV 0x18
#define IDX_DX_REFTOLERANCE 0x1A
#define IDX_XLATCHED 0x1C

#define IDX_LOW_WORD 0x60
#define IDX_HIGH_WORD 0x62
#define IDX_COVER_POS_LEN 0x64
#define IDX_COVER_DATA 0x66

#define IDX_REF_SWITCHES 0x7c
#define IDX_SMGP 0x7e

#define TMC428_READ 0x01
#define READ_REG 0x01

#define RM_RAMP 0
#define RM_SOFT 1
#define RM_VELOCITY 2
#define RM_HOLD 3

#define NO_REF 0x0f


/*********************************************************/
/** Register-Bit Definitions                             */
/*********************************************************/
//IDX_AGTAT_ALEAT
#define TMC428_AGTAT_ATHRESHOLD 0	//
#define TMC428_AGTAT_ISV0 12		//
#define TMC428_AGTAT_ISALEAT 16		// acceleration (a) is lower than or equal to (le) the threshold acceleration (at)
#define TMC428_AGTAT_ISAGTAT 20		// acceleration (a) is greater than (gt) a threshold acceleration (at)


//IDX_PULSEDIV_RAMPDIV
#define TMC428_PULRAM_USRS 0		// micro step resolution selection
#define TMC428_PULRAM_RAMPDIV 8		// 
#define TMC428_PULRAM_PULSEDIV 12	// 


//IDX_REFCONF_RM
#define TMC428_RM_RM 0				// 
#define TMC428_RM_REFCONF 8			// 
#define TMC428_RM_LP 16				// latched position


// Stepper Motor Global Parameter Register (IDX_SMGP)
#define TMC428_SMGPR_LSMD 0			// Last Stepper Motor Driver [1 Driver=0b00; 2 Drivers=0b01; 3 Drivers=0b10]
#define TMC428_SMGPR_P_NSCSS 2		// nSCS_S polarity
#define TMC428_SMGPR_P_SCKS 3		// SCK_S polarity
#define TMC428_SMGPR_P_PHAB 4		// polarity of the phase bits for the stepper motor
#define TMC428_SMGPR_P_FD 5			// polarity of the fast decay controlling bit
#define TMC428_SMGPR_P_DACAB 6		// polarity of the DAC bit vectors
#define TMC428_SMGPR_CS_COMIN 7		// use nSCS_S only or nSCS_S, nSCS2, nSCS3
#define TMC428_SMGPR_CLK2DIV 8		// clock frequency of the stepper motor driver chain clock signal SCK_S (8bit)
#define TMC428_SMGPR_CONTUPDT 16	// 
#define TMC428_SMGPR_REFMUX 20		// 
#define TMC428_SMGPR_MOT1R 21		// 




/*********************************************************/
/** Function Definitions                                 */
/*********************************************************/
void TMC428_Init(void);
int8_t TMC428_SetAMax(uint8_t Axis, uint16_t AMax);
void TMC428_SetMotorCurrent(uint8_t Motor, uint32_t *Current);

void TMC428_SendReceive(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char *Received);
void TMC428_Send(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void TMC428_SendLong(uint8_t Addr, int32_t Value);
void TMC428_SendReceiveArray(unsigned char *Received, unsigned char *Send);



#endif /* end __openDrive_TMC428_H */

