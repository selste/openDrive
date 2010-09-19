//
//  $Id: enc28j60.h 307 2008-11-07 03:50:38Z jcw $
//  $Revision: 307 $
//  $Author: jcw $
//  $Date: 2008-11-06 22:50:38 -0500 (Thu, 06 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/hardware/enc28j60.h $
//

#ifndef __ENC28J60_H__
#define __ENC28J60_H__

//
//
//
#include "FreeRTOS.h"
#include "../uip/uipopt.h"

//
// 
//
#define ENC28J60_Select()   GPIO0_FIOCLR = GPIO_IO_P7
#define ENC28J60_Deselect() GPIO0_FIOSET = GPIO_IO_P7

#define ENC28J60_Reset()    GPIO0_FIOCLR = GPIO_IO_P22
#define ENC28J60_Unreset()  GPIO0_FIOSET = GPIO_IO_P22

//
//
//
int   enc28j60Init (void);
void  enc28j60Deinit (void);
u16_t enc28j60Receive (void);
void  enc28j60Send (void);
signed portBASE_TYPE enc28j60WaitForData (portTickType delay);

//
//  ENC28J60 Opcodes 
//
#define	RCR                 (0x00<<5)		    	
#define RBM                 (0x01<<5)	| 0x1a 
#define WCR                 (0x02<<5)
#define WBM                 (0x03<<5) | 0x1a
#define BFS                 (0x04<<5)
#define	BFC                 (0x05<<5)
#define	SC                  (0xff)

#define BANK0 0
#define BANK1 1
#define BANK2 2
#define BANK3 3

//
//  Bank 0 Registers 
//
#define ERDPTL              0x00            // 
#define ERDPTH              0x01            // 
#define EWRPTL              0x02            // 
#define EWRPTH              0x03            // 
#define ETXSTL              0x04            // 
#define ETXSTH              0x05            // 
#define ETXNDL              0x06            // 
#define ETXNDH              0x07            // 
#define ERXSTL              0x08            // 
#define ERXSTH              0x09            // 
#define ERXNDL              0x0A            // 
#define ERXNDH              0x0B            // 
#define ERXRDPTL            0x0C            // 
#define ERXRDPTH            0x0D            // 
#define ERXWRPTL            0x0E            // 
#define ERXWRPTH            0x0F            // 
#define EDMASTL             0x10            // 
#define EDMASTH             0x11            // 
#define EDMANDL             0x12            // 
#define EDMANDH             0x13            // 
#define EDMADSTL            0x14            // 
#define EDMADSTH            0x15            // 
#define EDMACSL             0x16            // 
#define EDMACSH             0x17            // 
//                          0x18            // 
//                          0x19            // 
//                          0x1A            // 
#define EIE                 0x1B            // 
#define EIR                 0x1C            // 
#define ESTAT               0x1D            // 
#define ECON2               0x1E            // 
#define ECON1               0x1F            // 

//
//  Bank 1 Registers 
//
#define EHT0                0x00            // 
#define EHT1                0x01            // 
#define EHT2                0x02            // 
#define EHT3                0x03            // 
#define EHT4                0x04            // 
#define EHT5                0x05            // 
#define EHT6                0x06            // 
#define EHT7                0x07            // 
#define EPMM0               0x08            // 
#define EPMM1               0x09            // 
#define EPMM2               0x0A            // 
#define EPMM3               0x0B            // 
#define EPMM4               0x0C            // 
#define EPMM5               0x0D            // 
#define EPMM6               0x0E            // 
#define EPMM7               0x0F            // 
#define EPMCSL              0x10            // 
#define EPMCSh              0x11            // 
//                          0x12            // 
//                          0x13            // 
#define EPMOL               0x14            // 
#define EPMOH               0x15            // 
#define EWOLIE              0x16            // 
#define EWOLIR              0x17            // 
#define ERXFCON             0x18            // 
#define EPKTCNT             0x19            // 
//                          0x1A-0x1F       // See Bank 0 

//
//  Bank 2 Registers 
//
#define MACON1              0x00            
#define MACON2              0x01 
#define MACON3              0x02 
#define MACON4              0x03 
#define MABBIPG             0x04            
//                          0x05     
#define MAIPGL              0x06
#define MAIPGH              0x07 
#define MACLCON1            0x08 
#define MACLCON2            0x09 
#define MAMXFLL             0x0A 
#define MAMXFLH             0x0B 
//                          0x0C
#define MAPHSUP             0x0D 
//                          0x0E
//                          0x0F
//                          0x10 
#define MICON               0x11
#define MICMD               0x12
//                          0x13
#define MIREGADR            0x14 
//                          0x15 
#define MIWRL               0x16  
#define MIWRH               0x17  
#define MIRDL               0x18 
#define MIRDH               0x19        
//                          0x1A-0x1F       // See Bank 0 

//
//  Bank 3 Registers 
//
#define MAADR5              0x00            // 
#define MAADR6              0x01            // 
#define MAADR3              0x02            // 
#define MAADR4              0x03            // 
#define MAADR1              0x04            // 
#define MAADR2              0x05            // 
#define EBSTSD              0x06            // 
#define EBSTCON             0x07            // 
#define EBSTCSL             0x08            // 
#define EBSTCSH             0x09            // 
#define MISTAT              0x0A            // 
//                          0x0B            // 
//                          0x0C            // 
//                          0x0D            // 
//                          0x0E            // 
//                          0x0F            // 
//                          0x10            // 
//                          0x11            // 
#define EREVID              0x12            // 
//                          0x13            // 
//                          0x14            // 
#define ECOCON              0x15            // 
//                          0x16            // 
#define EFLOCON             0x17            // 
#define EPAUSL              0x18            // 
#define EPAUSH              0x19            // 
//                          0x1A-0x1F       // See Bank 0 

//
//  EIE bits 
//
#define	EIE_INTIE		        (1<<7)
#define	EIE_PKTIE		        (1<<6)
#define	EIE_DMAIE		        (1<<5)
#define	EIE_LINKIE		      (1<<4)
#define	EIE_TXIE		        (1<<3)
#define	EIE_TXERIE		      (1<<1)
#define	EIE_RXERIE		      (1<<0)

//
//  EIR bits 
//
#define	EIR_PKTIF		        (1<<6)
#define	EIR_DMAIF		        (1<<5)
#define	EIR_LINKIF		      (1<<4)
#define	EIR_TXIF		        (1<<3)
#define	EIR_TXERIF		      (1<<1)
#define	EIR_RXERIF		      (1<<0)
	
//
//  ESTAT bits 
//
#define	ESTAT_INT		        (1<<7)
#define ESTAT_BUFER         (1<<6)
#define ESTATE_RSVRD        (1<<5)
#define	ESTAT_LATECOL	      (1<<4)
#define ESTAT_UNIMP         (1<<3)
#define	ESTAT_RXBUSY	      (1<<2)
#define	ESTAT_TXABRT	      (1<<1)
#define	ESTAT_CLKRDY	      (1<<0)
	
//
//  ECON2 bits 
//
#define	ECON2_AUTOINC	      (1<<7)
#define	ECON2_PKTDEC	      (1<<6)
#define	ECON2_PWRSV		      (1<<5)
#define	ECON2_VRTP	      	(1<<4)
#define	ECON2_VRPS		      (1<<3)
	
//
//  ECON1 bits 
//
#define	ECON1_TXRST		      (1<<7)
#define	ECON1_RXRST		      (1<<6)
#define	ECON1_DMAST	      	(1<<5)
#define	ECON1_CSUMEN	      (1<<4)
#define	ECON1_TXRTS		      (1<<3)
#define	ECON1_RXEN		      (1<<2)
#define	ECON1_BSEL1		      (1<<1)
#define	ECON1_BSEL0		      (1<<0)
	
//
//  EWOLIE bits 
//
#define	EWOLIE_UCWOLIE	    (1<<7)
#define	EWOLIE_AWOLIE     	(1<<6)
#define	EWOLIE_PMWOLIE	    (1<<4)
#define	EWOLIE_MPWOLIE	    (1<<3)
#define	EWOLIE_HTWOLIE	    (1<<2)
#define	EWOLIE_MCWOLIE	    (1<<1)
#define	EWOLIE_BCWOLIE	    (1<<0)
	
//
//  EWOLIR bits 
//
#define	EWOLIR_UCWOLIF    	(1<<7)
#define	EWOLIR_AWOLIF	      (1<<6)
#define	EWOLIR_PMWOLIF	    (1<<4)
#define	EWOLIR_MPWOLIF	    (1<<3)
#define	EWOLIR_HTWOLIF	    (1<<2)
#define	EWOLIR_MCWOLIF	    (1<<1)
#define	EWOLIR_BCWOLIF	    (1<<0)
	
//
//  ERXFCON bits 
//
#define	ERXFCON_UCEN	      (1<<7)
#define	ERXFCON_ANDOR	      (1<<6)
#define	ERXFCON_CRCEN	      (1<<5)
#define	ERXFCON_PMEN	      (1<<4)
#define	ERXFCON_MPEN	      (1<<3)
#define	ERXFCON_HTEN	      (1<<2)
#define	ERXFCON_MCEN	      (1<<1)
#define	ERXFCON_BCEN	      (1<<0)
	
//
//  MACON1 bits 
//
#define	MACON1_LOOPBK	      (1<<4)
#define	MACON1_TXPAUS	      (1<<3)
#define	MACON1_RXPAUS	      (1<<2)
#define	MACON1_PASSALL	    (1<<1)
#define	MACON1_MARXEN	      (1<<0)
	
//
//  MACON2 bits 
//
#define	MACON2_MARST	      (1<<7)
#define	MACON2_RNDRST	      (1<<6)
#define	MACON2_MARXRST	    (1<<3)
#define	MACON2_RFUNRST	    (1<<2)
#define	MACON2_MATXRST	    (1<<1)
#define	MACON2_TFUNRST	    (1<<0)
	
//
//  MACON3 bits 
//
#define	MACON3_PADCFG2	    (1<<7)
#define	MACON3_PADCFG1	    (1<<6)
#define	MACON3_PADCFG0	    (1<<5)
#define	MACON3_TXCRCEN	    (1<<4)
#define	MACON3_PHDRLEN	    (1<<3)
#define	MACON3_HFRMEN	      (1<<2)
#define	MACON3_FRMLNEN	    (1<<1)
#define	MACON3_FULDPX	      (1<<0)
	
//
//  MACON4 bits 
//
#define	MACON4_DEFER	      (1<<6)
#define	MACON4_BPEN	      	(1<<5)
#define	MACON4_NOBKOFF	    (1<<4)
#define	MACON4_LONGPRE	    (1<<1)
#define	MACON4_PUREPRE	    (1<<0)
	
//
//  MAPHSUP bits 
//
#define	MAPHSUP_RSTRMII	    (1<<3)
	
//
//  MICON bits 
//
#define	MICON_RSTMII	      (1<<7)
	
//
//  MICMD bits 
//
#define	MICMD_MIISCAN	      (1<<1)
#define	MICMD_MIIRD		      (1<<0)

//
//  EBSTCON bits 
//
#define	EBSTCON_PSV2	      (1<<7)
#define	EBSTCON_PSV1	      (1<<6)
#define	EBSTCON_PSV0	      (1<<5)
#define	EBSTCON_PSEL	      (1<<4)
#define	EBSTCON_TMSEL1	    (1<<3)
#define	EBSTCON_TMSEL0	    (1<<2)
#define	EBSTCON_TME		      (1<<1)
#define	EBSTCON_BISTST	    (1<<0)

//
//  MISTAT bits 
//
#define	MISTAT_NVALID	      (1<<2)
#define	MISTAT_SCAN		      (1<<1)
#define	MISTAT_BUSY		      (1<<0)
	
//
//  ECOCON bits 
//
#define	ECOCON_COCON2     	(1<<2)
#define	ECOCON_COCON1	      (1<<1)
#define	ECOCON_COCON0	      (1<<0)
	
//
//  EFLOCON bits 
//
#define	EFLOCON_FULDPXS	    (1<<2)
#define	EFLOCON_FCEN1	      (1<<1)
#define	EFLOCON_FCEN0	      (1<<0)


//
//  PHY Registers 
//
#define PHCON1	            0x00
#define PHSTAT1	            0x01
#define PHID1	              0x02          
#define PHID2	              0x03
#define PHCON2	            0x10
#define PHSTAT2	            0x11
#define PHIE	              0x12
#define PHIR	              0x13
#define PHLCON	            0x14

//
//  PHY bits
//

//
// PHCON1 bits 
// 
#define	PHCON1_PRST		      (1<<15)
#define	PHCON1_PLOOPBK	    (1<<14)
#define	PHCON1_PPWRSV	      (1<<11)
#define	PHCON1_PDPXMD	      (1<<8)

//
// PHSTAT1 bits
//
#define	PHSTAT1_PFDPX	(1ul<<12)
#define	PHSTAT1_PHDPX	(1ul<<11)
#define	PHSTAT1_LLSTAT	(1ul<<2)
#define	PHSTAT1_JBSTAT	(1ul<<1)

//
// PHID2 bits 
//
#define	PHID2_PID24		(1ul<<15)
#define	PHID2_PID23		(1ul<<14)
#define	PHID2_PID22		(1ul<<13)
#define	PHID2_PID21		(1ul<<12)
#define	PHID2_PID20		(1ul<<11)
#define	PHID2_PID19		(1ul<<10)
#define	PHID2_PPN5		(1ul<<9)
#define	PHID2_PPN4		(1ul<<8)
#define	PHID2_PPN3		(1ul<<7)
#define	PHID2_PPN2		(1ul<<6)
#define	PHID2_PPN1		(1ul<<5)
#define	PHID2_PPN0		(1ul<<4)
#define	PHID2_PREV3		(1ul<<3)
#define	PHID2_PREV2		(1ul<<2)
#define	PHID2_PREV1		(1ul<<1)
#define	PHID2_PREV0		(1ul)

//
// PHCON2 bits 
//
#define	PHCON2_FRCLNK	          (1<<14)
#define	PHCON2_TXDIS	          (1<<13)
#define	PHCON2_JABBER	          (1<<10)
#define	PHCON2_HDLDIS	          (1<<8)

//
// PHSTAT2 bits
//
#define	PHSTAT2_TXSTAT	(1ul<<13)
#define	PHSTAT2_RXSTAT	(1ul<<12)
#define	PHSTAT2_COLSTAT	(1ul<<11)
#define	PHSTAT2_LSTAT	(1ul<<10)
#define	PHSTAT2_DPXSTAT	(1ul<<9)
#define	PHSTAT2_PLRITY	(1ul<<5)

//
// PHIE bits
//
#define	PHIE_PLNKIE		(1ul<<4)
#define	PHIE_PGEIE		(1ul<<1)

//
// PHIR bits
//
#define	PHIR_PLNKIF		(1ul<<4)
#define	PHIR_PGIF		(1ul<<2)

//
// PHLCON bits 
//
#define	PHLCON_LACFG3	(1ul<<11)
#define	PHLCON_LACFG2	(1ul<<10)
#define	PHLCON_LACFG1	(1ul<<9)
#define	PHLCON_LACFG0	(1ul<<8)
#define	PHLCON_LBCFG3	(1ul<<7)
#define	PHLCON_LBCFG2	(1ul<<6)
#define	PHLCON_LBCFG1	(1ul<<5)
#define	PHLCON_LBCFG0	(1ul<<4)
#define	PHLCON_LFRQ1	(1ul<<3)
#define	PHLCON_LFRQ0	(1ul<<2)
#define	PHLCON_STRCH	(1ul<<1)

#endif
