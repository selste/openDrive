//
//  $Id: mmc.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/mmc/mmc.h $
//

/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2003
 *
 *    File name   : mmc.h
 *    Description : define MMC module
 *
 *    History :
 *    1. Data        : July 1, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 42 $
**************************************************************************/

#ifndef _MMC_H_
#define _MMC_H_

#include "sysdefs.h"

// 
//  Time out definition for SD Read Time out ~100msec, Write Time out ~250ms
//
#define SD_READ_TIME_OUT          7     // [Hz]
#define SD_WRITE_TIME_OUT         3     // [HZ]

//
//
//
#define MMC_OK              0x00
#define MMC_IDLE_STATE      0x01
#define MMC_ERASE_RST       0x02
#define MMC_ILLEGAL_CMD     0x04
#define MMC_CRC_ERROR       0x08
#define MMC_ERASE_ERROR     0x10
#define MMC_ADD_ERROR       0x20
#define MMC_PARAM_ERROR     0x40

#define MMC_DATA_TOKEN      0xfe
#define MMC_DATA_ERR_TOKEN  0x1f
#define MMC_STOP_TRAN       0xfd

typedef enum
{
  MmcOk = 0, 
  MmcNoPresent, 
  MmcNoResponse, 
  MmcCardError, 
  MmcDataError,
  MmcUknowError, 
  MmmcParameterError, 
  MmcMiscompare
} 
mmcState_t;

typedef enum
{
  CMD0 = 0, // Resets the MultiMediaCard
  CMD1,     // Activates the card initialization process
  CMD9,     // Asks the selected card to send its card-specific data (CSD)
  CMD10,    // Asks the selected card to send its card identification (CID)
  CMD12,    // Stop transmission on multiple block read
  CMD13,    // Asks the selected card to send its status register
  CMD16,    // Selects a block length (in bytes) for all following block commands (read and write)
  CMD17,    // Reads a block of the size selected by the SET_BLOCKLEN command
  CMD18,    // Continuously transfers data blocks from card to host until interrupted by a Stop command or the requested number of data blocks transmitted
  CMD24,    // Writes a block of the size selected by the SET_BLOCKLEN command
  CMD25,    // Continuously writes blocks of data until a "Stop Tran" token or the requested number of blocks received
  CMD27,    // Programming of the programmable bits of the CSD
  CMD28,    // If the card has write protection features, this command sets the write protection bit of the addressed group. The properties of write protection are coded in the card specific data (WP_GRP_SIZE).
  CMD29,    // If the card has write protection features, this command clears the write protection bit of the addressed group
  CMD30,    // If the card has write protection features, this command asks the card to send the status of the write protection bits
  CMD32,    // Sets the address of the first sector of the erase group
  CMD33,    // Sets the address of the last sector in a continuous range within the selected erase group, or the address of a single sector to be selected for erase
  CMD34,    // Removes one previously selected sector from the erase selection
  CMD35,    // Sets the address of the first erase group within a range to be selected for erase
  CMD36,    // Sets the address of the last erase group within a continuous range to be selected for erase
  CMD37,    // Removes one previously selected erase group from the erase selection.
  CMD38,    // Erases all previously selected sectors
  CMD42,    // Used to set/reset the password or lock/unlock the card. The size of the Data Block is defined by the SET_BLOCK_LEN command
  CMD55,    // Notifies the card that the next command is an application specific command rather than a standard command.
  CMD56,    // Used either to transfer a Data Block to the card or to get a Data Block from the card for general purpose/application specific commands. The size of the Data Block is defined with SET_BLOCK_LEN command
  CMD58,    // Reads the OCR register of a card
  CMD59,    // Turns the CRC option on or off. A ‘1’ in the CRC option bit will turn the option on, a ‘0’ will turn it off
  ACMD41,   // Activates the card’s initialization process (Only for SD)
  CMD_END   // End of commands index
} 
mmcSpiCmdInd_t;

typedef enum
{
  MmcNoArg = 0,
  MmcBlockLen,
  MmcDataAdd, 
  MmcDummyWord
} 
mmcAgmType_t;

typedef enum
{
  MmcR1 = 0,
  MmcR1b,
  MmcR2,
  MmcR3
} 
mmcRespType_t;

typedef struct
{
  U8            txData;
  mmcAgmType_t  arg;
  mmcRespType_t resp;
} 
mmcCommands_t;

typedef enum
{
  DISKTYPE_MMC = 0, 
  DISKTYPE_SD, 
  DISKTYPE_UNKNOWN,
} 
diskType_e;

typedef struct
{
  U32  blockNumber;
  U32  blockSize;
  diskType_e diskType;
  BOOL writeProtect;
} 
mmcDiskStatus_t;

BOOL mmcWriteProtect (void);
BOOL mmcWaitReady (void);
BOOL mmcCardPresent (void);
mmcState_t mmcInitMedia (void);
mmcState_t mmcReadCardInfo (U8 *pData, mmcSpiCmdInd_t command);
mmcState_t mmcRead (U8 *pData, U32 address, U32 length);
mmcState_t mmcWrite (const U8 *pData, U32 address, U32 length);
mmcState_t mmcVerify (U8 *pData, U32 address, U32 length);
void mmcDumpCSD (void);
void mmcDecodeCSD (void);

#endif 
