#include "debug.h"

#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
//#include "semphr.h"

#include "spi.h"
#include "spi_port.h"
#include "sdcard.h"
#include "vbit.h"

unsigned char SDCard_Init(void) {

	unsigned char OCR[4], R1=0xFF, response[5], retry=0;

	/*** Send Card to Idle-State ***/
	do {
		SDCARD_SELECT;
		SDCard_Delay(4);
		SDCard_sendCommand(GO_IDLE_STATE, GO_IDLE_STATE_R, response, 0); //Send CMD0
        SDCARD_DESELECT;

		retry++;

		if(retry > 254) {
			debug_printf("SD init fail...\n");
			return SD_ERROR_INITFAIL;
		}//time out
 
	} while(response[0] != R1_IDLE_STATE); //Check if Card is in Idle-State


	/*** Check for SD-Card Type ***/		
	uint32_t arg = 0x01AA; //2.7-3.6V; check pattern = 0b10101010 => CRC 0x87	

    SDCARD_SELECT;
    SDCard_Delay(4);
	SDCard_sendCommand(SEND_IF_COND, SEND_IF_COND_R, response, arg); //SEND CMD8
    SDCARD_DESELECT;

	if(response[0] & R1_ILLEGAL_COMMAND) {
		//Ver1.X SD Memory Card or Not SD Memory Card
		
		debug_printf("Ver1.X SD Memory Card or Not SD Memory Card not supported...\n");
		return SD_ERROR_OLDCARD;
			
	} else {
		//Ver2.00 or later SD Memory Card		

		if(response[1] != 0x0
		   || response[2] != 0x0
		   || response[3] != 0x1
		   || response[4] != 0xAA) {
		   //Non-compatible voltage range or check pattern error
		   debug_printf("Non-compatible voltage range or check pattern error\n");
		   return SD_ERROR_WRONGVOLTAGE;
		}

		SDCARD_SELECT;
		SDCard_Delay(2);
		SDCard_ReadOCR(&R1,OCR); //SEND CMD58
        SDCARD_DESELECT;

		if(_VBIT_IS_CLEAR(OCR,SD_OCR_31_32)
		   || _VBIT_IS_CLEAR(OCR,SD_OCR_32_33)
		   || _VBIT_IS_CLEAR(OCR,SD_OCR_33_34)) {
		
			//Non-compatible voltage range
		   debug_printf("Non-compatible voltage range\n");
		   return SD_ERROR_WRONGVOLTAGE;
		}

		//Turn off IdleState		
		retry = 0;		
		do {	
			retry++;

			//prepare for acmd
            SDCARD_SELECT;
			SDCard_Delay(4);
			SDCard_sendCommand(APP_CMD, APP_CMD_R, response, 0); //SEND CMD55		

			//send acmd
			SDCard_Delay(4);
			arg = 0x0UL + (1UL<<SD_ACMD41_HCS); //HCS (host capacity support) | 1=> CRC=0x3B; 0=> CRC=0x72			
			SDCard_sendCommand(APP_SEND_OP_COND, APP_SEND_OP_COND_R, response, arg); //SEND ACMD41

			SDCard_Delay(500);
            SDCARD_DESELECT;
		
		} while(response[0] != 0x0 && retry < SD_CMD_TIMEOUT);

		if(retry >= SD_CMD_TIMEOUT) {
			//SDCard didn't turn of idle state
		   debug_printf("SDCard didn't turn of idle state\n");
		   return SD_ERROR_STAYIDLE;				
		}		

		SDCARD_SELECT;
		SDCard_Delay(4);
		SDCard_ReadOCR(&R1,OCR); //SEND CMD58	
        SDCARD_DESELECT;

		if(_VBIT_IS_SET(OCR,SD_OCR_CSS)) {
			//Ver2.00 or later High Capacity SD Memory Card
			debug_printf("Ver2.00 or later High Capacity SD Memory Card\n");

		} else {
			//Ver2.00 or later Standard Capacity SD Memory Card
			debug_printf("Ver2.00 or later Standard Capacity SD Memory Card\n");

		}
	}

	//SD-Card is now ready to use...
    SDCARD_SELECT;
	SDCard_Delay(4);
	SDCard_sendCommand(CRC_ON_OFF, CRC_ON_OFF_R, response, OFF); //disable CRC; deafault - CRC disabled in SPI mode
	SDCard_Delay(4);
	SDCard_sendCommand(SET_BLOCK_LEN, SET_BLOCK_LEN_R, response, 512); //set block size to 512
	SDCARD_DESELECT;

	debug_printf("SD initialisiert\n");

	return 0;



	return(0);

}

unsigned char SDCard_sendCommand(unsigned char cmd,
                             unsigned char response_type,
                             unsigned char *response,
                             unsigned long argument) {

	unsigned char tmp, response_length = 0, retry=0, crc;

	switch(cmd) {
		case SEND_IF_COND:
			crc = 0x87;
			break;
			
		case APP_SEND_OP_COND:
			if(argument == 0x0) {crc = 0x72;}
			else {crc = 0x3B;}
			break;
			
		default:
			crc = 0x95;
			break;
	}	
	
	
	// Send Cmd
	SDCARD_SPITRANSMIT((cmd & 0x3F) | 0x40); //send command, first two bits always '01'

	SDCARD_SPITRANSMIT(argument>>24);
	SDCARD_SPITRANSMIT(argument>>16);
	SDCARD_SPITRANSMIT(argument>>8);
	SDCARD_SPITRANSMIT(argument);

	SDCARD_SPITRANSMIT(crc);

	
    // Receive the respont from Card
    unsigned char timeout;
	do {
        tmp = SDCARD_SPITRANSMIT(0xFF);
		if(retry++ > SD_CMD_TIMEOUT) {
			timeout = 1;
			break; //time out error
		}
	} while((tmp&0x80)==0x80);


	switch(response_type) {
		case SD_RT_R1:
			response_length = 0;
			response[0] = tmp;
			break;
				
		case SD_RT_R1B:
			response_length = 1;
			break;

		case SD_RT_R2:
			response_length = 2;
			break;

		case SD_RT_R3:
			response_length = 5;
			break;

		case SD_RT_R6:
			response_length = 20;
			break;

		case SD_RT_R7:
			response_length = 5;
			break;

		default:
			response_length = 0;
			response[0] = tmp;
			break;
	}

	for(int i=0; i<response_length; i++) {
		response[i] = tmp;
        tmp = SDCARD_SPITRANSMIT(0xFF);
	}	
	
    SDCARD_SPICLEARFIFO();

	return tmp; //return state

}

void SDCard_Delay(uint32_t cnt) {

	for(int i=0; i<cnt; i++) {
		SDCARD_SPITRANSMIT(0xFF);
	}

	return;
}


unsigned char SDCard_ReadOCR(unsigned char *R1, unsigned char *OCR) {
	unsigned char response[5];

	SDCard_sendCommand(READ_OCR, READ_OCR_R, response, 0); //SEND CMD58	
	OCR[0] = response[4];
	OCR[1] = response[3];
	OCR[2] = response[2];
	OCR[3] = response[1];
	*R1 = response[0];

	if(*R1 & R1_ILLEGAL_COMMAND) {
		return(1);
	} else {
		return(0);
	}
}