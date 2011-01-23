#include "debug.h"

#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
//#include "semphr.h"

#include "TMC428.h"
#include "spi_port.h"
#include "spi.h"
#include "pwm_port.h"


//The following array contains:
//  -the first 64 bytes: configuration data of the TMC428 containing 
//   the structure of the SPI telegrams that are to be sent to the
//   drivers by the TMC428 
//  
//  -the second 64 bytes: microstep wave table
//   (sine wave, 1/4 period).
uint8_t driver_table[128] = {
	0x09,0x08,0x01,0x00,0x11,0x05,0x04,0x03,0x02,0x06,0x11,0x0d,0x0c,0x0b,0x0a,0x2e,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, 
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, 
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	 
	0x00,0x06,0x05,0x04,0x0a,0x09,0x0f,0x0e,0x0c,0x13,0x11,0x10,0x17,0x15,0x14,0x1a, 
	0x19,0x18,0x1e,0x1d,0x1c,0x22,0x21,0x20,0x27,0x25,0x24,0x2b,0x2a,0x29,0x28,0x2f, 
	0x2e,0x2d,0x2c,0x33,0x32,0x31,0x30,0x37,0x36,0x35,0x35,0x34,0x3b,0x3a,0x3a,0x39, 
	0x39,0x38,0x38,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3d,0x3d,0x3d,0x3c,0x3c,0x3c,0x3c 
};

//Motor configuration data
TMC428_MotorConfig_t MotorConfig[1]= {
	{
		1678,   //VMax
		1,      //AMax
		11,     //Pulsediv
		1       //Rampdiv
	}
};



/*************************************************************
  Function:  TMC428_Init
  Purpose: Initialize the TMC428
**************************************************************/ 
void TMC428_Init(void) {

    uint32_t cmd;

	//LPC_PINCON->PINSEL9 &= ~((0b11 << LPC_PINCON_PINSEL9_P4_28));		// P4.28 => GPIO Port
    //LPC_GPIO4->FIODIR |= (1 << 28);									// P4.28 => Output
	//LPC_PINCON->PINMODE9 |= (0b11 << LPC_PINCON_PINMODE9_P4_28);		// P4.28 => PullDown
	
	taskENTER_CRITICAL();

  	//Write the driver configuration data to the TMC428.
  	//This is the most important part of the intialization and should be done first of all!
	for(uint16_t i=128; i<256; i+=2) {
		TMC428_Send(i, 0, driver_table[i-127], driver_table[i-128]);
	}

  	//Set the "Stepper Motor Global Parameter"-Registers (SMGP).
	cmd = (0b00 << TMC428_SMGPR_LSMD)			// Use only 1 Motor-Driver
	    | (0 << TMC428_SMGPR_P_NSCSS)			// nSCS_S => active low
		| (0 << TMC428_SMGPR_P_SCKS)			// SCK_S polarity
		| (0 << TMC428_SMGPR_P_PHAB)			// changes the rotation direction
		| (1 << TMC428_SMGPR_P_FD)				// fast decay = active low
		| (0 << TMC428_SMGPR_P_DACAB)			// DAC bit vectors = active high
		| (0 << TMC428_SMGPR_CS_COMIN)			// Use only nSCS_S
		| (7 << TMC428_SMGPR_CLK2DIV)			// f_sck_s = f_clk / (2 * (clk2_div+1)) = 1MHz
		| (1 << TMC428_SMGPR_CONTUPDT)			// SPI_CONTINUOS_UPDATE
		| (0 << TMC428_SMGPR_REFMUX)			// Reference switch configuration
		| (0 << TMC428_SMGPR_MOT1R);			// Reference switch configuration
    TMC428_SendLong(IDX_SMGP, cmd);

  	//Set the coil current parameters
	cmd = (64 << TMC428_AGTAT_ATHRESHOLD)		// threshold
	    | (0b001 << TMC428_AGTAT_ISV0)			// current at rest => I = 12,5%
        | (0b000 << TMC428_AGTAT_ISALEAT)		// nominal motor current => I = 100%
        | (0b000 << TMC428_AGTAT_ISAGTAT);		// current during acceleration phases => I = 100%
	TMC428_SendLong(IDX_AGTAT_ALEAT, cmd);


  	//Set the pre-dividers and the microstep resolution
	cmd = (0b000 << TMC428_PULRAM_USRS)								// 64 µSteps
	    | (MotorConfig[0].RampDiv << TMC428_PULRAM_RAMPDIV)			// PulseDiv
		| (MotorConfig[0].PulseDiv << TMC428_PULRAM_PULSEDIV);		// RampDiv
	TMC428_SendLong(IDX_PULSEDIV_RAMPDIV|MOTOR0, cmd);
	


	//Now some examples of the driving parameters. This is highly application specific!
  	//Example for motor #0: RAMP mode
  	TMC428_SendLong(IDX_VMIN|MOTOR0, 1);  //Vmin = 1
  	TMC428_SendLong(IDX_VMAX|MOTOR0, MotorConfig[0].VMax);  //VMax
  	TMC428_SendLong(IDX_AMAX|MOTOR0, MotorConfig[0].AMax);  //AMax
  	
	TMC428_SetAMax(0, MotorConfig[0].AMax);

/*
	cmd = (0b00 << TMC428_RM_RM)			// RAMP_MODE
	    | (0b1111 << TMC428_RM_REFCONF);	// stop switches not active
*/ 	
	cmd = (0b10 << TMC428_RM_RM)			// VELOCITY_MODE
	    | (0b1111 << TMC428_RM_REFCONF);	// stop switches not active
  	TMC428_SendLong(IDX_REFCONF_RM|MOTOR0, cmd);

  	//Motor #0 can now be driven by setting the XTarget register (IDX_XTARGET) to the desired end position.
	//ATTENTION: The values in IDX_PMUL_PDIV depend on the AMax and Pulsediv/Rampdiv values and have to be
	//calculated using the algorithm given in the data sheet.
	//The values given here have been calculated using the Calc428.exe program on the TechLibCD.

	//TMC428_SendLong(IDX_XTARGET|MOTOR0, 1000000);
    //TMC428_SendLong(IDX_VTARGET|MOTOR0, 1346);
    TMC428_SendLong(IDX_VTARGET|MOTOR0, 2047);
	taskEXIT_CRITICAL();
}


/*******************************************************************
   Function: SetAMax
   Parameters: Axis: motor number (0, 1 oder 2)
               AMax: maximum acceleration value (1..2047)

   Return value: ---
   Purpose: Set the maximum acceleration value of one axis and
            calculate the PMUL/PDIV parameters accordingly
            (without the need for using floating point arithmetics).
********************************************************************/
int8_t TMC428_SetAMax(uint8_t Axis, uint16_t AMax) {
	int16_t pm;
	int8_t pd;
	uint8_t smda;
	uint8_t Read428[4], Write428[4];

	smda = Axis << 5;

	Write428[0] = ( smda | IDX_PULSEDIV_RAMPDIV | TMC428_READ);
	TMC428_SendReceiveArray(Read428, Write428);

	if(AMax & 0xF800) return -1;   //check if AMax is in the range 0...2047

	pm = AMax;
	//pd=4 +          rampdiv         -       pulsediv;
	pd = 4 + ( Read428 [ 2 ] & 0x0f ) - ( Read428 [ 2 ] >> 4 );

	if ( pm > 255 ) {
		do {
			if(pd<=0) {
				//(first) check for pd limit
				//write AMax, pm&pd to 428
				TMC428_Send( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
				TMC428_Send( smda | IDX_PMUL_PDIV, 0, 0x7f, 0 );
				
				return 1;
			}
			
			//shift (until hibyte of pm is zero)
			pm >>= 1;
			pd--;
		} while ( pm > 255  ); //check if hibyte of pm still "contains 1s"

		//MSB of lobyte must be "1", because we started with pm > 128 (at least one bit set in highbyte)
	} else {
		do {
			if(pd>=13) {
				//(first) check for pd limit
				//write AMax, pm&pd to 428
				TMC428_Send( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
				TMC428_Send( smda | IDX_PMUL_PDIV, 0, 0, 13 );
				
				return 2;
			}
			
			//shift until MSB of lobyte of pm is 1
			pm <<= 1;
			pd++;
		} while ( !( pm & 0x0080 ) ); //check if MSB of lobyte is (already) 1

		//MSB of lobyte must be "1" because we started with AMax > 1
	}

	//write AMax, pm&pd to 428
	TMC428_Send( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
	TMC428_Send( smda | IDX_PMUL_PDIV, 0, (uint8_t) pm, pd );
	return 0;
}


void TMC428_SetMotorCurrent(uint8_t Motor, uint32_t *Current) {

	switch(Motor) {
		case TMC428_MotorRA:
			if(*Current < 0) {
				*Current = 0;
			}
			else if(*Current > PWMPort_MR0) {
				*Current = PWMPort_MR0;
			}

			vPWMPort_SetMR(*Current);
			break;
	}

	return;
}


/*****************************************************************
  Function:  TMC428_SendReceive
  Purpose: Send four Bytes to the TMC428 and receives four Bytes
  Parameter:    a,b,c,d: the bytes that are to be written
                Received: Pointer to Receivebuffer (4 uchar)
*****************************************************************/ 
void TMC428_SendReceive(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char *Received) {

	TMC428_SPISTART;
	Received[0] = TMC428_SPITRANSMIT(a);
	Received[1] = TMC428_SPITRANSMIT(b);
	Received[2] = TMC428_SPITRANSMIT(c);
	Received[3] = TMC428_SPITRANSMIT(d);
	TMC428_SPISTOP;	

}

/*****************************************************************
  Function:  TMC428_Send
  Purpose: Send four Bytes to the TMC428
  Parameter:    a,b,c,d: the bytes that are to be written
*****************************************************************/ 
void TMC428_Send(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	//debug_printf("|0x%02X:0x%02X:0x%02X:0x%02X|\n", a, b, c, d);
	//return;
	
	TMC428_SPISTART;
	TMC428_SPITRANSMIT(a);
	TMC428_SPITRANSMIT(b);
	TMC428_SPITRANSMIT(c);
	TMC428_SPITRANSMIT(d);
	TMC428_SPISTOP;
}

/*******************************************************************
   Function: TMC428_SendLong
   Parameters: Addr: Register address (please see TMC428.h file).
               Value: Value to be written.

   Return value: ---

   Purpose: Write a long (24 bit) value into a TMC428 register.
********************************************************************/
void TMC428_SendLong(uint8_t Addr, int32_t Value) {

	Value = Value & 0x00FFFFFF; //only 24Bit are used!

	//unsigned char Received[4];	
    //TMC428_SendReceive(Addr, (Value >> 16), (Value >> 8), (Value >> 0), Received);

    TMC428_Send(Addr, (Value >> 16), (Value >> 8), (Value >> 0));
}

/*******************************************************************
   Function: TMC428_SendReceiveArray
   Parameters: Received: pointer to array of four bytes to be received
               Send: pointer to array of four bytes to send

   Return value: ---

   Purpose: SPI communication with the TMC428. Four bytes are sent
            and received.
********************************************************************/
void TMC428_SendReceiveArray(unsigned char *Received, unsigned char *Send) {

	TMC428_SendReceive(Send[0], Send[1], Send[2], Send[3], Received);
}


