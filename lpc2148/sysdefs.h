//
//  $Id: sysdefs.h 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/sysdefs.h $
//

#ifndef _SYSDEFS_H_
#define _SYSDEFS_H_

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

typedef unsigned char U8;
typedef signed char N8;
typedef unsigned short U16;
typedef signed short N16;
typedef unsigned int U32;
typedef signed int N32;
typedef int BOOL;

typedef volatile U8 REG8;
typedef volatile U16 REG16;
typedef volatile U32 REG32;

#define pREG8  (REG8 *)
#define pREG16 (REG16 *)
#define pREG32 (REG32 *)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y)((x)>(y)?(x):(y))
#define arrsizeof(x) ((sizeof (x))/(sizeof (x [0])))

//
//  Yuck.  Don't like this here, but what the heck... So, if no USB then no
//  USB serial or mass storage support
//
#ifndef CFG_USB
#undef CFG_USB_SER
#undef CFG_USB_MSC
#endif

//
//  If no USB serial, then no USB console support (warnings catch this later)
//
#ifndef CFG_USB_SER
#undef CFG_CONSOLE_USB
#endif

//
//  No SPI means no ENC28J60 (so no UIP), and no SPI EEPROM support
//
#ifndef CFG_SPI
#undef CFG_UIP
#undef CFG_M25LC512
#endif

//
//  No I2C, so no LM75 temp sensor or I2C EEPROM support
//
#ifndef CFG_I2C
#undef CFG_LM75
#undef CFG_AT24C1024
#endif

//
//  No UIP means no servers and no auto stuff
//
#ifndef CFG_UIP
#undef CFG_TELNETD
#undef CFG_HTTPD
#undef CFG_SNTP
#undef CFG_DHCP
#undef CFG_AUTO_UIP
#undef CFG_AUTO_DHCP
#undef CFG_AUTO_SNTP
#endif

//
//  Can't auto DHCP unless there's DHCP support, and auto requested
//
#if !defined CFG_DHCP || !defined CFG_AUTO_UIP
#undef CFG_AUTO_DHCP
#endif

//
//  We need an lvalue to pass to uipStart() to DHCP or not
//
#ifdef CFG_AUTO_DHCP
#define CFG_AUTO_DHCP_VALUE 1
#else
#define CFG_AUTO_DHCP_VALUE 0
#endif

//
//  If there's not RTC support, no point in SNTP
//
#ifndef CFG_RTC
#undef CFG_AUTO_SNTP
#endif

//
//  If the console is on UART1, we can't parse GPS data
//
#ifdef CFG_CONSOLE_UART1
#undef CFG_GPS
#endif

//
//  GPS uses floats, so no iprintf and friends when GPS is enabled
//
#ifdef CFG_GPS
#undef CFG_INTONLY
#endif

//
//  CFG_MDCODE is the actual memory dump code, and it's need by these guys...
//
#if defined CFG_I2C || defined CFG_M25LC512 || defined CFG_MD
#define CFG_MDCODE
#endif

//
//  What you can't and can't do with console combinations
//
#if !defined CFG_CONSOLE_USB && !defined CFG_CONSOLE_UART0 && !defined CFG_CONSOLE_UART1
#error "Must define CFG_CONSOLE_USB, CFG_CONSOLE_UART0 or CFG_CONSOLE_UART1"
#endif

#if defined CFG_CONSOLE_USB && (defined CFG_CONSOLE_UART0 || defined CFG_CONSOLE_UART1)
#error "Only one of CFG_CONSOLE_USB, CFG_CONSOLE_UART0 or CFG_CONSOLE_UART1 may be defined"
#endif
#if defined CFG_CONSOLE_UART0 && (defined CFG_CONSOLE_USB || defined CFG_CONSOLE_UART1)
#error "Only one of CFG_CONSOLE_USB, CFG_CONSOLE_UART0 or CFG_CONSOLE_UART1 may be defined"
#endif
#if defined CFG_CONSOLE_UART1 && (defined CFG_CONSOLE_USB || defined CFG_CONSOLE_UART0)
#error "Only one of CFG_CONSOLE_USB, CFG_CONSOLE_UART0 or CFG_CONSOLE_UART1 may be defined"
#endif

#endif
