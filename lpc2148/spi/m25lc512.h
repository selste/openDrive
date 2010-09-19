//
//  $Id: m25lc512.h 331 2008-11-09 16:59:47Z jcw $
//  $Revision: 331 $
//  $Author: jcw $
//  $Date: 2008-11-09 11:59:47 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/spi/m25lc512.h $
//

#ifndef _M25LC512_H_
#define _M25LC512_H_

//
//
//
#define M25LC512_SIZE       (65536)
#define M25LC512_SECTORSIZE (16384)
#define M25LC512_PAGESIZE     (128)
#define M25LC512_LASTSECTOR   (511)
#define M25LC512_LASTPAGE       (3)

//
//
//
typedef enum
{
  M25LC_OK = 0,     // Life is good
  M25LC_TIMEOUT,    // Timed out waiting for write to complete
  M25LC_NODEVICE,   // No device present (ID == 0xff or 0x00)
  M25LC_ADDRERR,    // Address out of range
  M25LC_SECTORERR,  // Sector > M25LC512_LASTSECTOR
  M25LC_PAGEERR,    // Page > M25LC512_LASTPAGE
  M25LC_WP,         // Write protected
  M25LC_WIP,        // Write in progress
  M25LC_LAST
}
m25lc_e;

//
//
//
void m25lc512Init (void);
const char *m25lc512Strerror (m25lc_e e);
m25lc_e m25lc512ReadID (U8 *id);
m25lc_e m25lc512DeepPowerDown (void);
m25lc_e m25lc512StatusRead (U8 *rdsr);
m25lc_e m25lc512StatusWrite (U8 wrsr);
m25lc_e m25lc512WriteEnable (void);
m25lc_e m25lc512WriteDisable (void);
m25lc_e m25lc512ErasePage (U32 page);
m25lc_e m25lc512EraseSector (U32 sector);
m25lc_e m25lc512EraseChip (void);
m25lc_e m25lc512SetAddress (U32 address);
int     m25lc512GetAddress (void);
m25lc_e m25lc512Read (U8 *buffer, U32 bufferLength);
m25lc_e m25lc512ReadAddress (U32 address, U8 *buffer, U32 bufferLength);
m25lc_e m25lc512Write (U8 *buffer, U32 bufferLength);
m25lc_e m25lc512WriteAddress (U32 address, U8 *buffer, U32 bufferLength);
m25lc_e m25lc512FillAddress (U32 address, U32 length, U8 fillValue);

#endif
