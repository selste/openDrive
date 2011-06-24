#ifndef __openDrive_SDCARD_H 
#define __openDrive_SDCARD_H


#define SDCARD_SELECT SSP0_SSEL_LOW
#define SDCARD_DESELECT SSP0_SSEL_HIGH

#define SDCARD_SPITRANSMIT xSSP0Port_Transmit
#define SDCARD_SPIRECEIVE xSSP0Port_Receive
#define SDCARD_SPICLEARFIFO xSSP0Port_ClearRxFifo

/*
#define SDCARD_SELECT SSP1_SSEL_LOW
#define SDCARD_DESELECT SSP1_SSEL_HIGH

#define SDCARD_SPITRANSMIT xSSP1Port_Transmit
#define SDCARD_SPIRECEIVE xSSP1Port_Receive
#define SDCARD_SPICLEARFIFO xSSP1Port_ClearRxFifo
*/

#define SD_RT_R1		1
#define SD_RT_R1B		2
#define SD_RT_R2		3
#define SD_RT_R3		4
#define SD_RT_R6		5
#define SD_RT_R7		6

#define GO_IDLE_STATE				0
#define GO_IDLE_STATE_R				SD_RT_R1
#define SEND_OP_COND				1
#define SEND_OP_COND_R				SD_RT_R1
#define SEND_IF_COND				8
#define SEND_IF_COND_R				SD_RT_R7
#define SEND_CSD					9
#define SEND_CSD_R					SD_RT_R6
#define STOP_TRANSMISSION			12
#define STOP_TRANSMISSION_R			SD_RT_R1B
#define SEND_STATUS					13
#define SEND_STATUS_R				SD_RT_R2
#define SET_BLOCK_LEN				16
#define SET_BLOCK_LEN_R				SD_RT_R1
#define READ_SINGLE_BLOCK			17
#define READ_MULTIPLE_BLOCKS		18
#define WRITE_SINGLE_BLOCK			24
#define WRITE_MULTIPLE_BLOCKS		25
#define ERASE_BLOCK_START_ADDR		32
#define ERASE_BLOCK_END_ADDR		33
#define ERASE_SELECTED_BLOCKS		38
#define APP_SEND_OP_COND			41
#define APP_SEND_OP_COND_R			SD_RT_R1
#define APP_CMD						55
#define APP_CMD_R					SD_RT_R1
#define READ_OCR					58
#define READ_OCR_R					SD_RT_R3
#define CRC_ON_OFF					59
#define CRC_ON_OFF_R				SD_RT_R1

#define R1_IDLE_STATE				0x01
#define R1_ERASE_RESET				0x02
#define R1_ILLEGAL_COMMAND			0x04
#define R1_COMMAND_CRC_ERROR		0x08
#define R1_ERASE_SEQUENCE_ERROR		0x10
#define R1_ADDRESS_ERROR			0x20
#define R1_PARAMETER_ERROR			0x40

#define R2_CARD_IS_LOCKED			0
#define R2_WP_ERASE_SKIP			1
#define R2_ERROR					2
#define R2_CC_ERROR					3
#define R2_CARD_ECC_FAILED			4
#define R2_WP_VIOLATION				5
#define R2_OUT_OF_RANGE				6
#define R2_IDLE_STATE				7
#define R2_ERASE_RESET				8
#define R2_ILLEGAL_COMMAND			9
#define R2_COMMAND_CRC_ERROR		10
#define R2_ERASE_SEQUENCE_ERROR		11
#define R2_ADDRESS_ERROR			12
#define R2_PARAMETER_ERROR			13


//OCR Register Definitions (S.74)
#define SD_OCR_LOWVOLTAGE			7
#define SD_OCR_27_28				15
#define SD_OCR_28_29				16
#define SD_OCR_29_30				17
#define SD_OCR_30_31				18
#define SD_OCR_31_32				19
#define SD_OCR_32_33				20
#define SD_OCR_33_34				21
#define SD_OCR_34_35				22
#define SD_OCR_35_36				23
#define SD_OCR_CSS					30
#define SD_OCR_BUSY					31

#define SD_CSD_STRUCTURE1			127
#define SD_CSD_STRUCTURE0			126

#define SD_ACMD41_HCS				30


/* Number of times to retry the probe cycle during initialization */
#define SD_INIT_TRY 50
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT_MAX 100
/* Hardcoded timeout for commands. 8 words, or 64 clocks. Do 10
* words instead */
#define SD_CMD_TIMEOUT 100


#define ON     1
#define OFF    0

/* ERROR Return-Codes */
#define SD_ERROR_INITFAIL			1
#define SD_ERROR_WRONGVOLTAGE		2
#define SD_ERROR_STAYIDLE			3
#define SD_ERROR_OLDCARD			4



unsigned char SDCard_Init(void);
void SDCard_Delay(uint32_t cnt);
unsigned char SDCard_sendCommand(unsigned char cmd,
                             unsigned char response_type,
                             unsigned char *response,
                             unsigned long argument);
unsigned char SDCard_ReadOCR(unsigned char *R1, unsigned char *OCR);



#endif /* end __openDrive_SDCARD_H */