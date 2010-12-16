#include <avr\io.h>
#include <stdint.h>


#define SS   PINB0  // X1B32  Butterfly: J400:P1
#define SCK  PINB1  // X1B28  Butterfly: J400:P2
#define MOSI PINB2  // X1A27  Butterfly: J400:P3
#define MISO PINB3  // X1B27  Butterfly: J400:P4

#define DDR_SPI  DDRB

#define F_CPU 8E6
#include <util/delay.h>


uint8_t incoming[4], outgoing[4];

#define SELECT_428()     PORTB &= ~(1 << SS)  //FILL IN HERE: Set the  nCS line of the TMC428 to low.
#define DESELECT_428()   PORTB |= (1 << SS)   //FILL IN HERE: Set the  nCS line of the TMC428 to high.
 

#include "Mini-TMCL.h"
#include "Globals.h"
#include "TMC428.h"			//Header file containing the register names of the TMC428


//The following array contains:
//  -the first 64 bytes: configuration data of the TMC428 containing 
//   the structure of the SPI telegrams that are to be sent to the
//   drivers by the TMC428 
//  
//  -the second 64 bytes: microstep wave table
//   (sine wave, 1/4 period).

uint8_t driver_table[128]={
0x09,0x08,0x01,0x00,0x11,0x05,0x04,0x03,0x02,0x06,0x11,0x0d,0x0c,0x0b,0x0a,0x2e,
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, 
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, 
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11, 
0x00,0x06,0x05,0x04,0x0a,0x09,0x0f,0x0e,0x0c,0x13,0x11,0x10,0x17,0x15,0x14,0x1a, 
0x19,0x18,0x1e,0x1d,0x1c,0x22,0x21,0x20,0x27,0x25,0x24,0x2b,0x2a,0x29,0x28,0x2f, 
0x2e,0x2d,0x2c,0x33,0x32,0x31,0x30,0x37,0x36,0x35,0x35,0x34,0x3b,0x3a,0x3a,0x39, 
0x39,0x38,0x38,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3d,0x3d,0x3d,0x3c,0x3c,0x3c,0x3c 
};

void Init_SPI(void)
{
	/* Set MISO, SCK, SS as output, all others input */
	DDR_SPI   = (1<<MOSI) | (1<<SCK) | (1<<SS);

	/* Enable SPI, Master, CPOL=1, CPHA=1 Setup (Falling) Sample (Rising), set clock rate fck/16 */
	SPCR      = (1<<SPE) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA)  | (1<<SPR0);
}


/**************************************************
  Function: uint8_t read_write_spi(uint8_t writebyte)
  Purpose: Write one byte to the SPI interface 
	         and read back one byte.
  Parameter: writebyte: byte to be written.
  Return value: byte that has been read back.
***************************************************/
uint8_t read_write_spi(uint8_t writebyte)
{
	//This function is controller specific and has to to
	//the following things:
  	// Send the contents of "writebyte" to the TCM428 via the SPI interface
  	// (MSB first!!!!!!!!!!!).
  	// Return the byte that has been written back from the TMC428.

  	SPDR = writebyte;
  	/* Wait for transmission complete */
  	while(!(SPSR & (1<<SPIF)));

  	return SPDR;
}



/*****************************************************************
  Function:  Send428
  Purpose: Send four Bytes to the TMC428
  Parameter:    a,b,c,d: the bytes that are to be written
*****************************************************************/ 
void Send428(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  	SELECT_428();			//Set the nCS line of the TMC428 low (controller specific).
  	read_write_spi(a);
  	read_write_spi(b);
  	read_write_spi(c);
  	read_write_spi(d);
  	DESELECT_428();	    	//Set the nCS line of the TMC428 high (controller specific).
}


/*************************************************************
  Function:  Send428
  Purpose: Initialize the TMC428
**************************************************************/ 
void Init428(void)
{
  	int16_t i;
  
  	//Write the driver configuration data to the TMC428.
  	//This is the most important part of the intialization and should be done first of all!
  	for(i=128; i<256; i+=2)
    	Send428(i, 0, driver_table[i-127], driver_table[i-128]);

  	//Set the "Stepper Motor Global Parameter"-Registers (SMGP).
  	//Set SPI_CONTINUOS_UPDATE  and PoFD, SPI_CLKDIV=7

  	Send428(IDX_SMGP, 0x01, 0x07, 0x20); // LSMD = 0 (which means 1 motor!)


  	//Set the coil current parameters (which is application specific)
  	Send428(IDX_AGTAT_ALEAT|MOTOR0, 0x00, 0x10, 0x40); //i_s_agtat/i_s_aleat/i_s_v0/a_threshold

  	//Set the pre-dividers and the microstep resolution (which is also application and motor specific)
	//pulsdiv/rampdiv/µStep
	Write428Bytes(IDX_PULSEDIV_RAMPDIV|MOTOR0, 0x00, (MotorConfig[0].PulseDiv<<4)|(MotorConfig[0].RampDiv & 0x0f), 0x06);

	//Now some examples of the driving parameters. This is highly application specific!

  	//Example for motor #0: RAMP mode
  	Send428(IDX_VMIN|MOTOR0, 0, 0, 1);  //Vmin = 1
  	Send428(IDX_VMAX|MOTOR0, 0, MotorConfig[0].VMax >> 8, MotorConfig[0].VMax & 0xff);  //VMax = MotorConfig[0].VMax
  	Send428(IDX_AMAX|MOTOR0, 0, MotorConfig[0].AMax >> 8, MotorConfig[0].AMax & 0xff);  //AMax = MotorConfig[0].AMax

	SetAMax(0, MotorConfig[0].AMax);
  	Send428(IDX_REFCONF_RM|MOTOR0, 0, NO_REF, RM_RAMP);  //RAMP mode, stop switches not active

  	//Motor #0 can now be driven by setting the XTarget register (IDX_XTARGET) to the desired end position.
	//ATTENTION: The values in IDX_PMUL_PDIV depend on the AMax and Pulsediv/Rampdiv values and have to be
	//calculated using the algorithm given in the data sheet.
	//The values given here have been calculated using the Calc428.exe program on the TechLibCD.

	Write428Long(IDX_XTARGET|MOTOR0,(1000000));

/*
  	//Example for motor #1: VELOCITY mode
  	Send428(IDX_VMIN|MOTOR1, 0, 0, 1);  //Vmin = 1
  	Send428(IDX_VMAX|MOTOR1, 0, MOT1_VMAX >> 8, MOT1_VMAX & 0xff);  //VMax = MOT1_VMAX (s. oben)
  	Send428(IDX_AMAX|MOTOR1, 0, MOT1_AMAX >> 8, MOT1_AMAX & 0xff);  //AMax = MOT1_AMAX
  	Send428(IDX_REFCONF_RM|MOTOR1, 0, NO_REF, RM_VELOCITY);  //VELOCITY mode, no stop switches

  	//Motor #1 can now be driven by setting the VTarget register (IDX_VTARGET) to the desired velocity.
	//The AMax value will be used to accelerate or decelerate the motor.
  
  	//Example for motor #2: HOLD mode
  	Send428(IDX_VMIN|MOTOR2, 0, 0, 1);  //Vmin = 1
  	Send428(IDX_VMAX|MOTOR2, 0, MOT2_VMAX >> 8, MOT2_VMAX & 0xff);  //VMax = MOT2_VMAX (s. oben)
  	Send428(IDX_REFCONF_RM|MOTOR2, 0, NO_REF, RM_HOLD);

  	//Der Motor 2 kann nun verfahren werden, indem das Register VActual (IDX_VACTUAL) auf die gewünschte
  	//Geschwindigkeit gesetzt wird. Diese Geschwindigkeit ist dann sofort wirksam, es wird nicht
  	//beschleunigt bzw. abgebremst.

	//Motor #2 can now be driven by setting the VActual register (IDX_VACTUAL) to the desired value.
	//The motor will then uses that velocity immediately, there is no accelerating or decelerating.
*/

}

/*******************************************************************
   Function: ReadWriteSPI4
   Parameters: incoming: pointer to array of four bytes
               outgoing: pointer to array of four bytes

   Return value: ---

   Purpose: SPI communication with the TMC428. Four bytes are sent
            and received.
********************************************************************/
void ReadWriteSPI4(uint8_t *incoming, uint8_t *outgoing)
{
  uint8_t i;

  for(i=0; i<4; i++)
  {
    SPDR=outgoing[i];
	while(!(SPSR & (1<<SPIF)));
    incoming[i]=SPDR;
  }
}

/*******************************************************************
   Function: ReadWrite428
   Parameters: Read: pointer to array of four bytes
               Write: pointer to array of four bytes

   Return value: ---

   Purpose: SPI communication with the TMC428. Four bytes are sent
            and received.
********************************************************************/
void ReadWrite428(uint8_t *Read, uint8_t *Write)
{
  SELECT_428();
  ReadWriteSPI4(Read, Write);
  DESELECT_428();
}


/*******************************************************************
   Function: Write428Long
   Parameters: Addr: Register address (please see TMC428.h file).
               Value: Value to be written.

   Return value: ---

   Purpose: Write a long (24 bit) value into a TMC428 register.
********************************************************************/
void Write428Long(uint8_t Addr, int32_t Value)
{
  outgoing[0]=Addr;
  outgoing[1]=BYTE2(Value);
  outgoing[2]=BYTE1(Value);
  outgoing[3]=BYTE0(Value);

  SELECT_428();
  ReadWriteSPI4(incoming, outgoing);
  DESELECT_428();
}


/*******************************************************************
   Function: Write428Int
   Parameters: Addr: Register address (please see TMC428.h file).
               Value: Value to be written.

   Return value: ---

   Purpose: Write an integer value (16 bit) into a TMC428 register.
   With registers containing less than 16 bits the highest bits
   will be lost.
********************************************************************/
void Write428Int(uint8_t Addr, int16_t Value)
{
  outgoing[0]=Addr;
  outgoing[1]=0;
  outgoing[2]=Value >> 8;
  outgoing[3]=Value & 0xff;

  SELECT_428();
  ReadWriteSPI4(incoming, outgoing);
  DESELECT_428();
}

/*******************************************************************
   Function: Write428Bytes
   Parameters: a, b, c, d: bytes to be written to the TMC428.

   Return value: ---

   Purpose: Write four bytes to the TMC428. The result can be
   found in the array "incoming".
********************************************************************/
void Write428Bytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  outgoing[0]=a;
  outgoing[1]=b;
  outgoing[2]=c;
  outgoing[3]=d;

  SELECT_428();
  ReadWriteSPI4(incoming, outgoing);
  DESELECT_428();
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
int8_t SetAMax(uint8_t Axis, uint16_t AMax)
{
  int16_t pm;
  int8_t pd;
  uint8_t smda;
  uint8_t Read428[4], Write428[4];

  smda = Axis << 5;

  Write428[0] = ( smda | IDX_PULSEDIV_RAMPDIV | TMC428_READ);
  ReadWrite428(Read428, Write428);

  if(AMax & 0xF800) return -1;   //check if AMax is in the range 0...2047

  pm = AMax;
  //pd=4 +          rampdiv         -       pulsediv;
  pd = 4 + ( Read428 [ 2 ] & 0x0f ) - ( Read428 [ 2 ] >> 4 );

  if ( pm > 255 )
  {
    do
    {
      if(pd<=0)
      {  //(first) check for pd limit
        //write AMax, pm&pd to 428
        Write428Bytes( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
        Write428Bytes( smda | IDX_PMUL_PDIV, 0, 0x7f, 0 );
        return 1;
      }
      //shift (until hibyte of pm is zero)
      pm >>= 1;
      pd--;
    } while ( pm > 255  ); //check if hibyte of pm still "contains 1s"
    //MSB of lobyte must be "1", because we started with pm > 128 (at least one bit set in highbyte)
  }
  else
  {
    do
    {
      if(pd>=13)
      { //(first) check for pd limit
        //write AMax, pm&pd to 428
        Write428Bytes( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
        Write428Bytes( smda | IDX_PMUL_PDIV, 0, 0, 13 );
        return 2;
      }
      //shift until MSB of lobyte of pm is 1
      pm <<= 1;
      pd++;
    } while ( !( pm & 0x0080 ) ); //check if MSB of lobyte is (already) 1
    //MSB of lobyte must be "1" because we started with AMax > 1
  }
  //write AMax, pm&pd to 428
  Write428Bytes( smda | IDX_AMAX, 0, (uint16_t) AMax >> 8, AMax & 0x00ff );
  Write428Bytes( smda | IDX_PMUL_PDIV, 0, (uint8_t) pm, pd );
  return 0;
}


int main (void) {

	Init_SPI();
	_delay_ms(1);

	Init428();
	_delay_ms(1);

	while (1) {
		_delay_ms(1);
	}
}


