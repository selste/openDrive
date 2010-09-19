//
//  $Id: a24c1024.h 302 2008-11-07 03:47:50Z jcw $
//  $Revision: 302 $
//  $Author: jcw $
//  $Date: 2008-11-06 22:47:50 -0500 (Thu, 06 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/i2c/a24c1024.h $
//

#ifndef _A24C1024_H_
#define _A24C1024_H_

//
//
//
#define A24C1024_ADDRESS  (0xa0)
#define A24C1024_SIZE     (131072)
#define A24C1024_PAGESIZE (256)

//
//
//
void a24c1024Init (void);
int a24c1024SetAddress (U32 address);
U32 a24c1024GetAddress (void);
int a24c1024Read (U8 *buffer, U32 bufferLength);
int a24c1024ReadAddress (U32 address, U8 *buffer, U32 bufferLength);
int a24c1024Write (U8 *buffer, U32 bufferLength);
int a24c1024WriteAddress (U32 address, U8 *buffer, U32 bufferLength);
int a24c1024FillAddress (U32 address, U32 length, U8 fillValue);

#endif
