//
//  $Id: spi.c 331 2008-11-09 16:59:47Z jcw $
//  $Revision: 331 $
//  $Author: jcw $
//  $Date: 2008-11-09 11:59:47 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/spi/spi.c $
//

#include "FreeRTOS.h"

#include "spi.h"

//
//
//
void spiInit (void)
{
  U32 spsr;

  //
  //  Setup SCK0, MISO0 and MOSI0.  SSEL0 (P0.7) is GPIO
  //
  PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P04_MASK | PCB_PINSEL0_P05_MASK | PCB_PINSEL0_P06_MASK | PCB_PINSEL0_P07_MASK)) | (PCB_PINSEL0_P04_SCK0 | PCB_PINSEL0_P05_MISO0 | PCB_PINSEL0_P06_MOSI0 | PCB_PINSEL0_P07_GPIO);

  //
  //  Set P0.7 as output, deselect by driving high
  //
  GPIO0_FIODIR |= GPIO_IO_P7;
  GPIO0_FIOSET  = GPIO_IO_P7;

  //
  //  Turn on SPI module power
  //
  SCB_PCONP |= SCB_PCONP_PCSPI0;

  //
  //  SPI configuration:
  //    Master mode
  //    Sample on 1st edge of SCK
  //    SCK is active high
  //    MSB out first
  //    No interrupts
  //
  SPI_SPCR = SPI_SPCR_MSTR | SPI_SPCR_CPHA1ST | SPI_SPCR_CPOLACTHIGH | SPI_SPCR_MSBF;
  
  //
  //  Reading the status register clears it
  //
  spsr = SPI_SPSR;

  //
  //  SCK0 = (PCLK = (CCLK/APB)) / 8 will be 6Mhz (putt putt putt...)
  //
  SPI_SPCCR = 8;
}

//
//
//
U8 spiPut (U8 valueIn)
{
  SPI_SPDR = valueIn;

  while (!(SPI_SPSR & SPI_SPSR_SPIF))
    ;

  return SPI_SPDR;
}
