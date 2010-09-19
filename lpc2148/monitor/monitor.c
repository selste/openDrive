//
//  $Id: monitor.c 352 2009-01-12 03:05:28Z jcw $
//  $Revision: 352 $
//  $Author: jcw $
//  $Date: 2009-01-11 22:05:28 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/monitor/monitor.c $
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/times.h>

#include "FreeRTOS.h"
#include "task.h"

#include "args.h"
#include "../fatfs/disk.h"
#include "../fatfs/ff.h"
#include "../fiq/fiq.h"
#include "../gps/gps.h"
#include "../i2c/a24c1024.h"
#include "../i2c/i2c.h"
#include "../i2c/lm75.h"
#include "../iap/iap.h"
#include "../kbd/kbd.h"
#include "../lcd/lcd.h"
#include "../leds/leds.h"
#include "../main.h"
#include "../pwm/pwm.h"
#include "../rtc/rtc.h"
#include "../sensors/sensors.h"
#include "../spi/m25lc512.h"
#include "../swi/swi.h"
#include "../timer/beep.h"
#include "../timer/timer.h"
#include "../uip/uiptask.h"
#include "../uip/uip/uip.h"
#include "../uip/uip/uip_arp.h"
#include "../uip/apps/telnetd/telnetd.h"
#include "../uip/apps/sntp/sntp.h"
#include "monitor.h"
#include "argsdispatch.h"

//
//
//
typedef struct abortDat_s
{
  unsigned int dummy;
  unsigned int sigil;
  unsigned int count;
  unsigned int type;
  unsigned int pc;
  unsigned int opcode;
  unsigned int cpsr;
  unsigned int lr;
  unsigned int sp;
  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;
  unsigned int r12;
  unsigned int stack [8];
}
__attribute__ ((packed)) abortDat_t;

//
//  Prototypes
//
static int monitorHelp (int argc, portCHAR **argv);
#ifdef CFG_MD
static int monitorMd (int argc, portCHAR **argv);
#endif
#ifdef CFG_ABORT
static int monitorAbortRegs (int argc, portCHAR **argv);
static int monitorAbortClear (int argc, portCHAR **argv);
static int monitorAbortDirty (int argc, portCHAR **argv);
static int monitorAbortUndef (int argc, portCHAR **argv);
static int monitorAbortPabort (int argc, portCHAR **argv);
static int monitorAbortDabort (int argc, portCHAR **argv);
#endif
#ifdef CFG_BEEP
static int monitorBeepOff (int argc, portCHAR **argv);
static int monitorBeepOn (int argc, portCHAR **argv);
static int monitorBeepMHALL (int argc, portCHAR **argv);
static int monitorBeepSMOTW (int argc, portCHAR **argv);
#endif
#ifdef CFG_AT24C1024
static int monitorEEIAddr (int argc, portCHAR **argv);
static int monitorEEIRead (int argc, portCHAR **argv);
static int monitorEEIReadAddr (int argc, portCHAR **argv);
static int monitorEEIWrite (int argc, portCHAR **argv);
static int monitorEEIWriteAddr (int argc, portCHAR **argv);
static int monitorEEIFillAddr (int argc, portCHAR **argv);
#endif
#ifdef CFG_M25LC512
static int monitorEESAddr (int argc, portCHAR **argv);
static int monitorEESRead (int argc, portCHAR **argv);
static int monitorEESReadAddr (int argc, portCHAR **argv);
static int monitorEESWrite (int argc, portCHAR **argv);
static int monitorEESWriteAddr (int argc, portCHAR **argv);
static int monitorEESFillAddr (int argc, portCHAR **argv);
static int monitorEESEraseChip (int argc, portCHAR **argv);
static int monitorEESEraseSector (int argc, portCHAR **argv);
static int monitorEESErasePage (int argc, portCHAR **argv);
static int monitorEESWriteEnable (int argc, portCHAR **argv);
static int monitorEESWriteDisable (int argc, portCHAR **argv);
static int monitorEESReadID (int argc, portCHAR **argv);
static int monitorEESPowerDown (int argc, portCHAR **argv);
static int monitorEESStatusRead (int argc, portCHAR **argv);
static int monitorEESStatusWrite (int argc, portCHAR **argv);
#endif
#ifdef CFG_FATFS
static int monitorFileChmod (int argc, portCHAR **argv);
static int monitorFileConcat (int argc, portCHAR **argv);
static int monitorFileCp (int argc, portCHAR **argv);
static int monitorFileCpCon (int argc, portCHAR **argv);
static int monitorFileDf (int argc, portCHAR **argv);
static int monitorFileInit (int argc, portCHAR **argv);
static int monitorFileLs (int argc, portCHAR **argv);
static int monitorFileMkfs (int argc, portCHAR **argv);
static int monitorFileMount (int argc, portCHAR **argv);
static int monitorFileMkdir (int argc, portCHAR **argv);
static int monitorFileMv (int argc, portCHAR **argv);
static int monitorFileRm (int argc, portCHAR **argv);
static int monitorFileRmdir (int argc, portCHAR **argv);
static int monitorFileSync (int argc, portCHAR **argv);
static int monitorFileThruPut (int argc, portCHAR **argv);
#endif
#ifdef CFG_FIQ
static int monitorFIQClear (int argc, portCHAR **argv);
static int monitorFIQCount (int argc, portCHAR **argv);
static int monitorFIQOff (int argc, portCHAR **argv);
static int monitorFIQOn (int argc, portCHAR **argv);
#endif
#ifdef CFG_GPS
static int monitorGPSShow (int argc, portCHAR **argv);
static int monitorGPSStart (int argc, portCHAR **argv);
static int monitorGPSStop (int argc, portCHAR **argv);
#endif
#ifdef CFG_I2C
static int monitorI2CRead (int argc, portCHAR **argv);
static int monitorI2CWrite (int argc, portCHAR **argv);
static int monitorI2CWriteRead (int argc, portCHAR **argv);
static int monitorI2CDump (int argc, portCHAR **argv);
static int monitorI2CErrno (int argc, portCHAR **argv);
#endif
#ifdef CFG_IAP
static int monitorIAPFSS (int argc, portCHAR **argv);
static int monitorIAPSTOA (int argc, portCHAR **argv);
static int monitorIAPFill (int argc, portCHAR **argv);
static int monitorIAPErase (int argc, portCHAR **argv);
static int monitorIAPBlank (int argc, portCHAR **argv);
static int monitorIAPID (int argc, portCHAR **argv);
static int monitorIAPVer(int argc, portCHAR **argv);
static int monitorIAPISP (int argc, portCHAR **argv);
#endif
#ifdef CFG_KBD
static int monitorKbdGet (int argc, portCHAR **argv);
static int monitorKbdStart (int argc, portCHAR **argv);
static int monitorKbdStop (int argc, portCHAR **argv);
#endif
#ifdef CFG_LCD
static int monitorLCDStart (int argc, portCHAR **argv);
static int monitorLCDStop (int argc, portCHAR **argv);
static int monitorLCDClear (int argc, portCHAR **argv);
static int monitorLCDGotoXY (int argc, portCHAR **argv);
static int monitorLCDMessage (int argc, portCHAR **argv);
#endif
#ifdef CFG_LED
static int monitorLEDStart (int argc, portCHAR **argv);
static int monitorLEDStop (int argc, portCHAR **argv);
#endif
#ifdef CFG_LM75
static int monitorLM75Init (int argc, portCHAR **argv);
static int monitorLM75Mode (int argc, portCHAR **argv);
static int monitorLM75Addr (int argc, portCHAR **argv);
static int monitorLM75ReRead (int argc, portCHAR **argv);
static int monitorLM75Temp (int argc, portCHAR **argv);
static int monitorLM75Config (int argc, portCHAR **argv);
static int monitorLM75Thyst (int argc, portCHAR **argv);
static int monitorLM75Tos (int argc, portCHAR **argv);
#endif
#ifdef CFG_MEM
static int monitorMemTask (int argc, portCHAR **argv);
static int monitorMemMap (int argc, portCHAR **argv);
static int monitorMemAlloc (int argc, portCHAR **argv);
static int monitorMemRealloc (int argc, portCHAR **argv);
static int monitorMemFree (int argc, portCHAR **argv);
static int monitorMemList (int argc, portCHAR **argv);
#endif
#ifdef CFG_MISC
static int monitorMiscPorts (int argc, portCHAR **argv);
static int monitorMiscSizeof (int argc, portCHAR **argv);
#endif
#ifdef CFG_PWM
static int monitorPWMDuty (int argc, portCHAR **argv);
static int monitorPWMFrequency (int argc, portCHAR **argv);
static int monitorPWMWidth (int argc, portCHAR **argv);
#endif
#ifdef CFG_RTC
static int monitorRTCGet (int argc, portCHAR **argv);
static int monitorRTCSet (int argc, portCHAR **argv);
static int monitorRTCAlarm (int argc, portCHAR **argv);
static int monitorRTCPeriodic (int argc, portCHAR **argv);
#endif
#ifdef CFG_SENSORS
static int monitorSensorsShow (int argc, portCHAR **argv);
static int monitorSensorsStart (int argc, portCHAR **argv);
static int monitorSensorsStop (int argc, portCHAR **argv);
#endif
#ifdef CFG_SWI
static int monitorSWISetAsm (int argc, portCHAR **argv);
static int monitorSWIOnAsm (int argc, portCHAR **argv);
static int monitorSWIOffAsm (int argc, portCHAR **argv);
static int monitorSWIToggleAsm (int argc, portCHAR **argv);
static int monitorSWISetC (int argc, portCHAR **argv);
static int monitorSWIOnC (int argc, portCHAR **argv);
static int monitorSWIOffC (int argc, portCHAR **argv);
static int monitorSWIToggleC (int argc, portCHAR **argv);
#endif
#ifdef CFG_UIP
static int monitorUIPDHCP (int argc, portCHAR **argv);
#ifdef CFG_TELNETD
static int monitorUIPDrop (int argc, portCHAR **argv);
#endif
static int monitorUIPGW (int argc, portCHAR **argv);
static int monitorUIPIP (int argc, portCHAR **argv);
static int monitorUIPMAC (int argc, portCHAR **argv);
static int monitorUIPNM (int argc, portCHAR **argv);
#ifdef CFG_SNTP
static int monitorUIPSNTP (int argc, portCHAR **argv);
#endif
static int monitorUIPStart (int argc, portCHAR **argv);
static int monitorUIPStop (int argc, portCHAR **argv);
static int monitorUIPStats (int argc, portCHAR **argv);
#ifdef CFG_SNTP
static int monitorUIPTO (int argc, portCHAR **argv);
#endif
#endif
#ifdef CFG_WDT
static int monitorWDTTest (int argc, portCHAR **argv);
static int monitorWDTStatus (int argc, portCHAR **argv);
static int monitorWDTClear (int argc, portCHAR **argv);
#endif
static int monitorVersion (int argc, portCHAR **argv);

//
//
//
#ifdef CFG_ABORT
static const commandList_t commandListAbort [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                 "'help' has no parameters" },
  { "regs",     0,  0, CMDTYPE_FUNCTION,  { monitorAbortRegs   }, "Print abort registers",          "'regs' has no parameters" },
  { "clear",    0,  0, CMDTYPE_FUNCTION,  { monitorAbortClear  }, "Clear abort registers",          "'clear' has no parameters" },
  { "dirty",    0,  0, CMDTYPE_FUNCTION,  { monitorAbortDirty  }, "Dirty sigil flag",               "'dirty' has no parameters" },
  { "undef",    0,  0, CMDTYPE_FUNCTION,  { monitorAbortUndef  }, "Execute undefined instruction",  "'undef' has no parameters" },
  { "pabort",   0,  0, CMDTYPE_FUNCTION,  { monitorAbortPabort }, "Cause prefetch abort",           "'pabort' has no parameters" },
  { "dabort",   0,  0, CMDTYPE_FUNCTION,  { monitorAbortDabort }, "Cause data abort",               "'dabort' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                             NULL },
};
#endif

#ifdef CFG_BEEP
static const commandList_t commandListBeep [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "off",      0,  0, CMDTYPE_FUNCTION,  { monitorBeepOff     }, "Turn beeper off",              "'off' has no parameters" },
  { "on",       1,  1, CMDTYPE_FUNCTION,  { monitorBeepOn      }, "Start <hz> frequency beep",    "'on [60..20000]'" },
  { "mhall",    0,  0, CMDTYPE_FUNCTION,  { monitorBeepMHALL   }, "Play Mary Had a Little Lamb",  "'mhall' has no parameters" },
  { "smotw",    0,  0, CMDTYPE_FUNCTION,  { monitorBeepSMOTW   }, "Play Smoke On The Water",      "'smotw' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_AT24C1024
static const commandList_t commandListEEI [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "a",        1,  1, CMDTYPE_FUNCTION,  { monitorEEIAddr     }, "Set EEPROM r/w address",       "'ee <address>'" },
  { "r",        0,  1, CMDTYPE_FUNCTION,  { monitorEEIRead     }, "Read from current address",    "'r <# bytes>'" },
  { "ra",       1,  2, CMDTYPE_FUNCTION,  { monitorEEIReadAddr }, "Read EEPROM",                  "'ra <address> <# bytes>'" },
  { "w",        1, 16, CMDTYPE_FUNCTION,  { monitorEEIWrite    }, "Write to current address",     "'w <byte> [<byte> [...<byte>]]'" },
  { "wa",       2, 17, CMDTYPE_FUNCTION,  { monitorEEIWriteAddr}, "Write EEPOM",                  "'wa <address> <byte> [<byte> [...<byte>]]'" },
  { "fa",       3,  3, CMDTYPE_FUNCTION,  { monitorEEIFillAddr }, "Fill EEPOM",                   "'fa <address> <len> <byte>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_M25LC512
static const commandList_t commandListEES [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp            }, "This help list",               "'help' has no parameters" },
  { "a",        1,  1, CMDTYPE_FUNCTION,  { monitorEESAddr         }, "Set EEPROM r/w address",       "'ee <address>'" },
  { "r",        0,  1, CMDTYPE_FUNCTION,  { monitorEESRead         }, "Read from current address",    "'r <# bytes>'" },
  { "ra",       1,  2, CMDTYPE_FUNCTION,  { monitorEESReadAddr     }, "Read EEPROM",                  "'ra <address> <# bytes>'" },
  { "w",        1, 16, CMDTYPE_FUNCTION,  { monitorEESWrite        }, "Write to current address",     "'w <byte> [<byte> [...<byte>]]'" },
  { "wa",       2, 17, CMDTYPE_FUNCTION,  { monitorEESWriteAddr    }, "Write EEPOM",                  "'wa <address> <byte> [<byte> [...<byte>]]'" },
  { "fa",       3,  3, CMDTYPE_FUNCTION,  { monitorEESFillAddr     }, "Fill EEPOM",                   "'fa <address> <len> <byte>'" },
  { "ce",       0,  0, CMDTYPE_FUNCTION,  { monitorEESEraseChip    }, "Erase EEPROM",                 "'ce' has no parameters" },
  { "se",       1,  1, CMDTYPE_FUNCTION,  { monitorEESEraseSector  }, "Erase sector",                 "'se <0..3>'" },
  { "pe",       1,  1, CMDTYPE_FUNCTION,  { monitorEESErasePage    }, "Erase page",                   "'pe <0..511>'" },
  { "wren",     0,  0, CMDTYPE_FUNCTION,  { monitorEESWriteEnable  }, "Enable writes to EEPROM",      "'wren' has no parameters" },
  { "wrdi",     0,  0, CMDTYPE_FUNCTION,  { monitorEESWriteDisable }, "Disable writes to EEPROM",     "'wrdi' has no parameters" },
  { "id",       0,  0, CMDTYPE_FUNCTION,  { monitorEESReadID       }, "Read device ID",               "'id' has no parameters" },
  { "dpd",      0,  0, CMDTYPE_FUNCTION,  { monitorEESPowerDown    }, "Deep power-down device",       "'dpd' has no parameters" },
  { "rdsr",     0,  0, CMDTYPE_FUNCTION,  { monitorEESStatusRead   }, "Read status register",         "'rdsr' has no parameters" },
  { "wrsr",     1,  1, CMDTYPE_FUNCTION,  { monitorEESStatusWrite  }, "Write status register",        "'wrsr <0..255>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL                   }, NULL,                           NULL },
};
#endif

#ifdef CFG_FATFS
static const commandList_t commandListFile [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "chmod",    2,  2, CMDTYPE_FUNCTION,  { monitorFileChmod   }, "Change file mode",             "'chmod <+w|-w> <[path]filename>'" },
  { "concat",   2,  2, CMDTYPE_FUNCTION,  { monitorFileConcat  }, "Concatenate 1st to 2nd file",  "'concat <[path]1st_filename> <[path]2nd_filename>'" },
  { "cp",       1,  2, CMDTYPE_FUNCTION,  { monitorFileCp      }, "Copy a file",                  "'cp <[path]old_filename> <[path]new_filename>'" },
  { "cpcon",    1,  1, CMDTYPE_FUNCTION,  { monitorFileCpCon   }, "Copy console input to file",   "'cpcon <[path]filename>' (ctrl-d exits and saves)" },
  { "df",       0,  0, CMDTYPE_FUNCTION,  { monitorFileDf      }, "File system status & info",    "'df' has no parameters" },
  { "init",     0,  0, CMDTYPE_FUNCTION,  { monitorFileInit    }, "Initialize disk subsystem",    "'init' has no parameters" },
  { "ls",       0,  1, CMDTYPE_FUNCTION,  { monitorFileLs      }, "Display directory",            "'ls [optional path]'" },
  { "mkdir",    1,  1, CMDTYPE_FUNCTION,  { monitorFileMkdir   }, "Create directory",             "'mkdir <[path]directory'" },
  { "mkfs",     0,  0, CMDTYPE_FUNCTION,  { monitorFileMkfs    }, "Create a FAT filesystem",      "'mkfs' has no parameters" },
  { "mount",    0,  0, CMDTYPE_FUNCTION,  { monitorFileMount   }, "Mount disk",                   "'mount' has no parameters" },
  { "mv",       2,  2, CMDTYPE_FUNCTION,  { monitorFileMv      }, "Rename a file or directory",   "'mv <[path]old_filename> <[path]new_filename>'" },
  { "rm",       1,  1, CMDTYPE_FUNCTION,  { monitorFileRm      }, "Delete file",                  "'rm <[path]filename>'" },
  { "rmdir",    1,  1, CMDTYPE_FUNCTION,  { monitorFileRmdir   }, "Delete directory",             "'rmdir <[path]directory>'" },
  { "sync",     0,  0, CMDTYPE_FUNCTION,  { monitorFileSync    }, "Sync filesystem",              "'sync' has no parameters" },
  { "thruput",  1,  1, CMDTYPE_FUNCTION,  { monitorFileThruPut }, "Measure MMC/SD throughput",    "'thruput <normal|noints|high|suspendall>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_FIQ
static const commandList_t commandListFIQ [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                 "'help' has no parameters" },
  { "clear",    0,  0, CMDTYPE_FUNCTION,  { monitorFIQClear    }, "Clear FIQ interrupt counter",    "'clear' has no parameters" },
  { "count",    0,  0, CMDTYPE_FUNCTION,  { monitorFIQCount    }, "Show number of FIQ interrupts",  "'show' has no parameters" },
  { "off",      0,  0, CMDTYPE_FUNCTION,  { monitorFIQOff      }, "Disable FIQ interrupt",          "'off' has no parameters" },
  { "on",       0,  0, CMDTYPE_FUNCTION,  { monitorFIQOn       }, "Enable FIQ interrupt",           "'on' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                             NULL },
};
#endif

#ifdef CFG_GPS
static const commandList_t commandListGPS [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "show",     0,  0, CMDTYPE_FUNCTION,  { monitorGPSShow     }, "Display GPS data",             "'show' has no parameters" },
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorGPSStart    }, "Start GPS task",               "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorGPSStop     }, "Stop GPS task",                "'stop' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_I2C
static const commandList_t commandListI2C [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                     "'help' has no parameters" },
  { "r",        2,  2, CMDTYPE_FUNCTION,  { monitorI2CRead     }, "Read from I2C device",               "'r <address> <# bytes>'" },
  { "w",        2, 17, CMDTYPE_FUNCTION,  { monitorI2CWrite    }, "Write to I2C device",                "'w <address> <byte> [<byte> [...<byte>]]'" },
  { "wr",       2, 18, CMDTYPE_FUNCTION,  { monitorI2CWriteRead}, "Write to then read from I2C device", "'wr <address> <byte> [<byte> [...<byte>]] <# bytes to read>'" },
  { "dump",     0,  0, CMDTYPE_FUNCTION,  { monitorI2CDump     }, "Dump I2C Debug Buffer",              "'dump' has no parameters" },
  { "errno",    0,  0, CMDTYPE_FUNCTION,  { monitorI2CErrno    }, "Display i2cErrno value",             "'errno' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                                 NULL },
};
#endif

#ifdef CFG_IAP
static const commandList_t commandListIAP [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                     "'help' has no parameters" },
  { "fss",      0,  0, CMDTYPE_FUNCTION,  { monitorIAPFSS      }, "Find safe sector",                   "'fss' has no parameters" },
  { "stoa",     1,  1, CMDTYPE_FUNCTION,  { monitorIAPSTOA     }, "Convert sector to address for 'md'", "'fss' has no parameters" },
  { "fill",     2,  2, CMDTYPE_FUNCTION,  { monitorIAPFill     }, "Fill sector with byte",              "'fill <sector> <byte>'" },
  { "erase",    1,  1, CMDTYPE_FUNCTION,  { monitorIAPErase    }, "Erase sector",                       "'erase <sector>'" },
  { "blank",    1,  1, CMDTYPE_FUNCTION,  { monitorIAPBlank    }, "Blank check sector",                 "'blank <sector>'" },
  { "id",       0,  0, CMDTYPE_FUNCTION,  { monitorIAPID       }, "Read part ID",                       "'id' has no parameters" },
  { "ver",      0,  0, CMDTYPE_FUNCTION,  { monitorIAPVer      }, "Read boot loader version",           "'ver' has no parameters" },
  { "isp",      0,  0, CMDTYPE_FUNCTION,  { monitorIAPISP      }, "Restart into ISP bootloader",        "'isp' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                                 NULL },
};
#endif

#ifdef CFG_KBD
static const commandList_t commandListKBD [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "get",      0,  0, CMDTYPE_FUNCTION,  { monitorKbdGet      }, "Get character from keyboard",  "'start' has no parameters" },
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorKbdStart    }, "Start keyboard task",          "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorKbdStop     }, "End keyboard task",            "'stop' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_LCD
static const commandList_t commandListLCD [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",         "'help' has no parameters" },
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorLCDStart    }, "Start LCD task",         "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorLCDStop     }, "End LCD task",           "'stop' has no parameters" },
  { "clear",    0,  0, CMDTYPE_FUNCTION,  { monitorLCDClear    }, "Clear display",          "'clear' has no parameters" },
  { "gotoxy",   2,  2, CMDTYPE_FUNCTION,  { monitorLCDGotoXY   }, "Set cursor position",    "'gotoxy <column> <line>'" },
  { "msg",      1,  1, CMDTYPE_FUNCTION,  { monitorLCDMessage  }, "Send message to LCD",    "'msg <text>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                     NULL },
};
#endif

#ifdef CFG_LED
static const commandList_t commandListLED [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",         "'help' has no parameters" },
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorLEDStart    }, "Start LED task",         "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorLEDStop     }, "End LED task",           "'stop' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                     NULL },
};
#endif

#ifdef CFG_LM75
static const commandList_t commandListLM75 [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "init",     0,  0, CMDTYPE_FUNCTION,  { monitorLM75Init    }, "Initialize LM75",              "'init' has no parameters" },
  { "mode",     1,  1, CMDTYPE_FUNCTION,  { monitorLM75Mode    }, "Set LM75 mode",                "'mode <value>'" },
  { "addr",     1,  1, CMDTYPE_FUNCTION,  { monitorLM75Addr    }, "Set LM75 address",             "'addr <value>'" },
  { "reread",   0,  0, CMDTYPE_FUNCTION,  { monitorLM75ReRead  }, "Re-read last register",        "'reread' has no parameters" },
  { "temp",     0,  0, CMDTYPE_FUNCTION,  { monitorLM75Temp    }, "Read LM75 temperature",        "'temp' has no parameters" },
  { "config",   0,  1, CMDTYPE_FUNCTION,  { monitorLM75Config  }, "Read part ID",                 "'id' has no parameters" },
  { "thyst",    0,  1, CMDTYPE_FUNCTION,  { monitorLM75Thyst   }, "Read or set THYST register",   "'thyst [value]'" },
  { "tos",      0,  1, CMDTYPE_FUNCTION,  { monitorLM75Tos     }, "Read or set TOS register",     "'tos [value]'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_MEM
static const commandList_t commandListMem [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "task",     0,  0, CMDTYPE_FUNCTION,  { monitorMemTask     }, "Show FreeRTOS task memory",    "'task' has no parameters" },
  { "map",      0,  0, CMDTYPE_FUNCTION,  { monitorMemMap      }, "Show various addresses",       "'map' has no parameters" },
  { "alloc",    2,  2, CMDTYPE_FUNCTION,  { monitorMemAlloc    }, "Allocate memory",              "'alloc <slot> <size>'" },
  { "realloc",  2,  2, CMDTYPE_FUNCTION,  { monitorMemRealloc  }, "Reallocate memory",            "'realloc <slot> <size>'" },
  { "free",     1,  1, CMDTYPE_FUNCTION,  { monitorMemFree     }, "Free memory",                  "'free <slot>'" },
  { "list",     0,  0, CMDTYPE_FUNCTION,  { monitorMemList     }, "List memory",                  "'list' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_MISC
static const commandList_t commandListMisc [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "ports",    0,  0, CMDTYPE_FUNCTION,  { monitorMiscPorts   }, "Display port registers",       "'ports' has no parameters" },
  { "sizeof",   0,  0, CMDTYPE_FUNCTION,  { monitorMiscSizeof  }, "Sizeof() variable data types", "'sizeof' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_PWM
static const commandList_t commandListPWM [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                   "'help' has no parameters" },
  { "duty",     1,  1, CMDTYPE_FUNCTION,  { monitorPWMDuty     }, "Set PWM duty cycle",               "'duty <0..100>'" },
  { "freq",     1,  1, CMDTYPE_FUNCTION,  { monitorPWMFrequency}, "Set PWM frequency",                "'freq <1..48000000>'" },
  { "width",    1,  1, CMDTYPE_FUNCTION,  { monitorPWMWidth    }, "Set PWM width (in microseconds)",  "'width <1..48000000>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                               NULL },
};
#endif

#ifdef CFG_RTC
static const commandList_t commandListRTC [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "get",      0,  0, CMDTYPE_FUNCTION,  { monitorRTCGet      }, "Display system date/time",     "'get' has no parameters" },
  { "set",      1,  2, CMDTYPE_FUNCTION,  { monitorRTCSet      }, "Set system date/time",         "'set <gps|YYYY/MM/DD HH:MM:SS>'" },
  { "alarm",    0,  2, CMDTYPE_FUNCTION,  { monitorRTCAlarm    }, "Set date/time for alarm",      "'alarm <off|YYYY/MM/DD HH:MM:SS>'" },
  { "periodic", 0,  1, CMDTYPE_FUNCTION,  { monitorRTCPeriodic }, "Alarm every minute change",    "'periodic <on|off>'" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_SENSORS
static const commandList_t commandListSensors [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "show",     0,  0, CMDTYPE_FUNCTION,  { monitorSensorsShow }, "Display sensor data",          "'show' has no parameters" },
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorSensorsStart}, "Start sensors task",           "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorSensorsStop }, "Stop sensors task",            "'stop' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_SWI
static const commandList_t commandListSWI [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "aset",     1,  1, CMDTYPE_FUNCTION,  { monitorSWISetAsm   }, "Set LED2 state (asm)",         "'aset <on|of>'" },
  { "aon",      0,  0, CMDTYPE_FUNCTION,  { monitorSWIOnAsm    }, "Turn LED2 on (asm)",           "'aon' has no parameters" },
  { "aoff",     0,  0, CMDTYPE_FUNCTION,  { monitorSWIOffAsm   }, "Turn LED2 off (asm)",          "'aoff' has no parameters" },
  { "atoggle",  0,  0, CMDTYPE_FUNCTION,  { monitorSWIToggleAsm}, "Toggle LED2 state (asm)",      "'atoggle' has no parameters" },
  { "cset",     1,  1, CMDTYPE_FUNCTION,  { monitorSWISetC     }, "Set LED2 state (C)",           "'cset <on|off>'" },
  { "con",      0,  0, CMDTYPE_FUNCTION,  { monitorSWIOnC      }, "Turn LED2 on (C)",             "'con' has no parameters" },
  { "coff",     0,  0, CMDTYPE_FUNCTION,  { monitorSWIOffC     }, "Turn LED2 off (C)",            "'coff' has no parameters" },
  { "ctoggle",  0,  0, CMDTYPE_FUNCTION,  { monitorSWIToggleC  }, "Toggle LED2 state (C)",        "'ctoggle' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_UIP
static const commandList_t commandListUIP [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "dhcp",     0,  1, CMDTYPE_FUNCTION,  { monitorUIPDHCP     }, "Display/set DHCP state",       "'dhcp [<on|off>]'" },
#ifdef CFG_TELNETD
  { "drop",     0,  1, CMDTYPE_FUNCTION,  { monitorUIPDrop     }, "Drop telnet client connection","'drop' has no parameters" },
#endif
  { "gw",       0,  1, CMDTYPE_FUNCTION,  { monitorUIPGW       }, "Display/set gateway address",  "'gw [<a.b.c.d>]'" },
  { "ip",       0,  1, CMDTYPE_FUNCTION,  { monitorUIPIP       }, "Display/set IP address",       "'ip [<a.b.c.d>]'" },
  { "mac",      0,  1, CMDTYPE_FUNCTION,  { monitorUIPMAC      }, "Display/set MAC address",      "'mac [<xx:yy:zz:aa:bb:cc>]'" },
  { "nm",       0,  1, CMDTYPE_FUNCTION,  { monitorUIPNM       }, "Display/set netmask address",  "'nm [<a.b.c.d>]'" },
#ifdef CFG_SNTP
  { "sntp",     0,  1, CMDTYPE_FUNCTION,  { monitorUIPSNTP     }, "Request SNTP time",            "'sntp [<set>|<a.b.c.d>]'" },
#endif
  { "start",    0,  0, CMDTYPE_FUNCTION,  { monitorUIPStart    }, "Start UIP stack",              "'start' has no parameters" },
  { "stop",     0,  0, CMDTYPE_FUNCTION,  { monitorUIPStop     }, "Stop UIP stack",               "'stop' has no parameters" },
  { "stats",    0,  0, CMDTYPE_FUNCTION,  { monitorUIPStats    }, "Display uIP status",           "'stats' has no parameters" },
#ifdef CFG_SNTP
  { "to",       0,  1, CMDTYPE_FUNCTION,  { monitorUIPTO       }, "Display/set time offset",      "'to [offset_in_seconds]'" },
#endif
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

#ifdef CFG_WDT
static const commandList_t commandListWDT [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",               "'help' has no parameters" },
  { "test",     0,  0, CMDTYPE_FUNCTION,  { monitorWDTTest     }, "Test watchdog",                "'test' has no parameters" },
  { "status",   0,  0, CMDTYPE_FUNCTION,  { monitorWDTStatus   }, "Display RSIR register",        "'status' has no parameters" },
  { "clear",    0,  0, CMDTYPE_FUNCTION,  { monitorWDTClear    }, "Clear RSIR status",            "'rsir' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                           NULL },
};
#endif

static const commandList_t commandList [] =
{
  { "help",     0,  0, CMDTYPE_FUNCTION,  { monitorHelp        }, "This help list",                 "'help' has no parameters" },
#ifdef CFG_ABORT
  { "abort",    1,  0, CMDTYPE_CMDLIST,   { commandListAbort   }, "Read/clear abort registers",     "'abort help' for help list" },
#endif
#ifdef CFG_BEEP
  { "beep",     1,  0, CMDTYPE_CMDLIST,   { commandListBeep    }, "Beep related functions",         "'beep help' for help list" },
#endif
#ifdef CFG_AT24C1024
  { "eei",      1,  0, CMDTYPE_CMDLIST,   { commandListEEI     }, "Read/write I2C EEPROM",          "'eei help' for help list" },
#endif
#ifdef CFG_M25LC512
  { "ees",      1,  0, CMDTYPE_CMDLIST,   { commandListEES     }, "Read/write SPI EEPROM",          "'ees help' for help list" },
#endif
#ifdef CFG_FATFS
  { "file",     1,  0, CMDTYPE_CMDLIST,   { commandListFile    }, "Various file related commands",  "'file help' for help list" },
#endif
#ifdef CFG_FIQ
  { "fiq",      1,  0, CMDTYPE_CMDLIST,   { commandListFIQ     }, "Demonstrate FIQ functions",      "'fiq help' for help list" },
#endif
#ifdef CFG_GPS
  { "gps",      1,  0, CMDTYPE_CMDLIST,   { commandListGPS     }, "GPS related functions",          "'gps help' for help list" },
#endif
#ifdef CFG_I2C
  { "i2c",      1,  0, CMDTYPE_CMDLIST,   { commandListI2C     }, "Perform I2C commands",           "'i2c help' for help list" },
#endif
#ifdef CFG_IAP
  { "iap",      1,  0, CMDTYPE_CMDLIST,   { commandListIAP     }, "Perform IAP commands",           "'iap help' for help list" },
#endif
#ifdef CFG_KBD
  { "kbd",      1,  0, CMDTYPE_CMDLIST,   { commandListKBD     }, "Various keyboard commands",      "'kbd help' for help list" },
#endif
#ifdef CFG_LCD
  { "lcd",      1,  0, CMDTYPE_CMDLIST,   { commandListLCD     }, "Various LCD commands",           "'lcd help' for help list" },
#endif
#ifdef CFG_LED
  { "led",      1,  0, CMDTYPE_CMDLIST,   { commandListLED     }, "Various LED commands",           "'led help' for help list" },
#endif
#ifdef CFG_LM75
  { "lm75",     1,  0, CMDTYPE_CMDLIST,   { commandListLM75    }, "Read/set LM75 temp sensor",      "'lm75 help' for help list" },
#endif
#ifdef CFG_MD
  { "md",       0,  2, CMDTYPE_FUNCTION,  { monitorMd          }, "Display memory",                 "'md [address [length]]'" },
#endif
#ifdef CFG_MEM
  { "mem",      1,  0, CMDTYPE_CMDLIST,   { commandListMem     }, "Various memory functions",       "'mem help' for help list" },
#endif
#ifdef CFG_MISC
  { "misc",     1,  0, CMDTYPE_CMDLIST,   { commandListMisc    }, "Miscellaneous stuff",            "'misc help' for help list" },
#endif
#ifdef CFG_PWM
  { "pwm",      1,  0, CMDTYPE_CMDLIST,   { commandListPWM     }, "Demonstrate PWM functions",      "'pwm help' for help list" },
#endif
#ifdef CFG_RTC
  { "rtc",      1,  0, CMDTYPE_CMDLIST,   { commandListRTC     }, "Demonstrate RTC functions",      "'rtc help' for help list" },
#endif
#ifdef CFG_SENSORS
  { "sensors",  1,  0, CMDTYPE_CMDLIST,   { commandListSensors }, "Sensors related functions",      "'sensors help' for help list" },
#endif
#ifdef CFG_SWI
  { "swi",      1,  0, CMDTYPE_CMDLIST,   { commandListSWI     }, "Demonstrate SWI functions",      "'swi help' for help list" },
#endif
#ifdef CFG_UIP
  { "uip",      1,  0, CMDTYPE_CMDLIST,   { commandListUIP     }, "Various uIP commands",           "'uip help' for help list" },
#endif
#ifdef CFG_WDT
  { "wdt",      1,  0, CMDTYPE_CMDLIST,   { commandListWDT     }, "Manipulate watchdog timer",      "'wdt help' for help list" },
#endif
  { "version",  0,  0, CMDTYPE_FUNCTION,  { monitorVersion     }, "Display version information",    "'version' has no parameters" },
  { NULL,       0,  0, CMDTYPE_FUNCTION,  { NULL               }, NULL,                             NULL },
};

#ifdef CFG_FATFS
static FATFS fatfs;
static FILINFO fileInfo;
#endif
#ifdef CFG_UIP
#ifdef CFG_DHCP
static int uipDHCPC = TRUE;
#else
static int uipDHCPC = FALSE;
#endif
#endif
commandList_t *activeCommandList = NULL;

//
//  External variables
//
extern unsigned int __abort_dat;
extern unsigned long __start_of_text__;
extern unsigned long __end_of_text__;
extern unsigned long __start_of_startup__;
extern unsigned long __end_of_startup__;
extern unsigned long __start_of_prog__;
extern unsigned long __end_of_prog__;
extern unsigned long __start_of_rodata__;
extern unsigned long __end_of_rodata__;
extern unsigned long __start_of_glue7__;
extern unsigned long __end_of_glue7__;
extern unsigned long __data_beg__;
extern unsigned long __data_end__;
extern unsigned long __bss_beg__;
extern unsigned long __bss_end__;
extern unsigned long __heap_max;
extern unsigned long __heap_beg;
extern unsigned long __heap_end;
extern unsigned long __stack_end__;
extern unsigned long __stack_beg_und;
extern unsigned long __stack_end_und;
extern unsigned long __stack_beg_abt;
extern unsigned long __stack_end_abt;
extern unsigned long __stack_beg_fiq;
extern unsigned long __stack_end_fiq;
extern unsigned long __stack_beg_irq;
extern unsigned long __stack_end_irq;
extern unsigned long __stack_beg_svc;
extern unsigned long __stack_end_svc;
extern unsigned long __stack_beg_sys;
extern unsigned long __stack_end_sys;
extern xTaskHandle taskHandles [TASKHANDLE_LAST];

//
//
//
#if defined CFG_LCD || defined CFG_UIP
static int isDecimalString (char *s)
{
  while (*s)
    if (!isdigit (*s++))
      return 0;

  return 1;
}
#endif

#ifdef CFG_FATFS
//
//  Recursion is our friend...  fileInfo is global to avoid eating stack space
//
static FRESULT scan_files_ex (char *path, int *total_size, int *total_files, int *total_dirs)
{
  DIR dirs;
  FRESULT res;

  if ((res = f_opendir (&dirs, path)) == FR_OK) 
  {
    int i = strlen (path);

    while (((res = f_readdir (&dirs, &fileInfo)) == FR_OK) && fileInfo.fname [0]) 
    {
      if (fileInfo.fattrib & AM_DIR) 
      {
        *total_dirs += 1;
        *(path + i) = '/'; 
        strcpy (path + i + 1, &fileInfo.fname [0]);
        res = scan_files_ex (path, total_size, total_files, total_dirs);
        *(path + i) = '\0';

        if (res != FR_OK) 
          return res;
      } 
      else 
      {
        *total_files += 1;
        *total_size += fileInfo.fsize;
      }
    }
  }

  return res;
}

static FRESULT scan_files (char *path, int *total_size, int *total_files, int *total_dirs)
{
  *total_size = 0;
  *total_files = 0;
  *total_dirs = 0;

  return scan_files_ex (path, total_size, total_files, total_dirs);
}

//
//  These two really ought to be in the FatFS code
//
U32 get_fattime ()
{
  U32 tmr;
  time_t now;
  struct tm tm;

  now = time (NULL);
  localtime_r (&now, &tm);

  tmr = 0
    | ((tm.tm_year - 80) << 25)
    | ((tm.tm_mon + 1)   << 21)
    | (tm.tm_mday        << 16)
    | (tm.tm_hour        << 11)
    | (tm.tm_min         << 5)
    | (tm.tm_sec         >> 1);

  return tmr;
}

//
//  Functions newlib doesn't know about (but should)
//
void _sync  (void);
int  _mkdir (const char *path, mode_t mode);
int  _chmod (const char *path, mode_t mode);

void sync  (void);
int  chmod (const char *path, mode_t mode);

void sync (void)
{
  _sync ();
}

int mkdir (const char *path, mode_t mode)
{
  return _mkdir (path, mode);
}

int chmod (const char *path, mode_t mode)
{
  return _chmod (path, mode);
}
#endif

#if defined CFG_MD || defined CFG_MEM || defined CFG_IAP || defined CFG_UIP || defined CFG_SNTP || defined CFG_I2C || defined CFG_LM75 || defined CFG_AT24C1024 || defined CFG_M25LC512
//
//
//
static int getNumber (char *s, unsigned int *result)
{
  unsigned int value;
  unsigned int mustBeHex = FALSE;
  int sgn = 1;
  const unsigned char hexToDec [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15};

  if (!s)
    return 0;

  if ((strlen (s) > 2) && (!strncmp (s, "0x", 2) || !strncmp (s, "0X", 2)))
  {
    mustBeHex = TRUE;
    s += 2;
  }

  if (!mustBeHex && *s && (*s == '-'))
  {
    sgn = -1;
    s++;
  }

  for (value = 0; *s; s++)
  {
    if (mustBeHex && isxdigit (*s))
      value = (value << 4) | hexToDec [toupper (*s) - '0'];
    else if (isdigit (*s))
      value = (value * 10) + (*s - '0');
    else
    {
      printf ("Malformed number.  Must be decimal number, or hex value preceeded by '0x'\n");
      return 0;
    }
  }

  if (!mustBeHex)
    value *= sgn;

  *result = value;

  return 1;
}
#endif

#ifdef CFG_UIP
static int getIPAddress (char *s, unsigned int *ipAddress)
{
  char *p = s;
  char *t = NULL;
  int i;

  for (*ipAddress = 0, i = 0; i < 4; i++)
  {
    unsigned int ipOctet;

    if (i != 3)
    { 
      if ((t = index (p, '.')) && (t != p))
        *t = '\0';
      else
      { 
        printf ("Incorrectly formatted IP address\n");
        return 0;
      }
    }

    if (!*p)
    { 
      printf ("Missing octet in IP address\n");
      return 0;
    }

    if (!isDecimalString (p))
    {
      printf ("Bad octet value in IP address: \"%s\"\n", p);
      return 0;
    }

    getNumber (p, &ipOctet);

    if (ipOctet > 255)
    {
      printf ("Illegal octet value: \"%s\"\n", p);
      return 0;
    }

    *ipAddress = (*ipAddress << 8) | ipOctet;
    p = t + 1;
  }

  return 1;
}
#endif

#ifdef CFG_UIP
static int atoh (char *s, unsigned int *r)
{
  unsigned int i;
  unsigned int l;
  char buffer [(l = strlen (s)) + 3];

  for (i = 0; i < l; i++)
    if (!isxdigit (s [i]))
      return 0;

  strcpy (buffer, "0x");
  strcat (buffer, s);

  return getNumber (buffer, r);
}
#endif

#ifdef CFG_MDCODE
//
//
//
static int monitorDumpMemory (unsigned int displayAddress, unsigned int mask, unsigned int address, int length)
{
  unsigned char *buffer;
  int i;

  if (!length)
  {
    printf ("Error: monitorDumpMemory() passed 0 for length\n");
    return address;
  }

  for (buffer = (unsigned char *) address, i = 0; i < length; i += 16)
  {
    unsigned int l;
    unsigned int j;

    if (i)
      printf ("\n");

    printf ("%08x: ", (displayAddress + i) & mask);

    if ((length - i) < 16)
      l = length & 15;
    else
      l = 16;

    for (j = 0; j < 16; j++)
    {
      if (j < l)
        printf ("%02x ", buffer [i+j]);
      else
        printf ("   ");
    }

    printf ("  ");

    for (j = 0; j < l; j++)
    {
      unsigned char c = buffer [i+j];

      if (c < 32 || c > 127)
        c = '.';

      printf ("%c", c);
    }
  }

  printf ("\n");

  address += length;

  return address;
}
#endif

//
//
//
static int monitorHelp (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  unsigned int i;
  int t;
  int longestCmd;
  portCHAR spaces [32];

  memset (spaces, ' ', sizeof (spaces));

  for (longestCmd = 0, i = 0; activeCommandList [i].command; i++)
    if ((t = strlen (activeCommandList [i].command)) > longestCmd)
      longestCmd = t;

  spaces [longestCmd] = '\0';

  for (i = 0; activeCommandList [i].command; i++)
  {
    const commandList_t *cl = &activeCommandList [i];

    printf ("%s%s -- %s\n", cl->command, &spaces [strlen (cl->command)], cl->description);
  }

  printf ("\nUse '<command> ?' for details on parameters to command\n");

  return 0;
}

#ifdef CFG_MD
//
//
//
static int monitorMd (int argc, portCHAR **argv)
{
  static unsigned int address = 0x00000000;
  unsigned int length = 256;

  if ((argc >= 1))
  {
    if (!getNumber (argv [0], &address))
      return 0;

    if (argc == 2)
      if (!getNumber (argv [1], &length))
        return 0;
  }

  address = monitorDumpMemory (address, 0xffffffff, address, length);

  return 0;
}
#endif

#ifdef CFG_ABORT
//
//
//
static int monitorAbortRegs (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  abortDat_t *ad = (abortDat_t *) &__abort_dat;

  printf ("contents=%s, sigil=0x%08x, count=%d\n", (ad->sigil == 0xdeadc0de) ? "probable" : "invalid", ad->sigil, ad->count);
  printf ("abort type=%s\n", (ad->type == 0) ? "undefined instruction" : (ad->type == 1) ? "prefetch abort" : (ad->type == 2) ? "data abort" : "unknown");
  printf ("pc=0x%08x, opcode=0x%08x\n", ad->pc, ad->opcode);
  printf ("cpsr=0x%08x, sp=0x%08x, lr=0x%08x\n", ad->cpsr, ad->sp, ad->lr);
  printf ("r0=0x%08x, r1=0x%08x, r2=0x%08x, r3=0x%08x\n", ad->r0, ad->r1, ad->r2, ad->r3);
  printf ("r4=0x%08x, r5=0x%08x, r6=0x%08x, r7=0x%08x\n", ad->r4, ad->r5, ad->r6, ad->r7);
  printf ("r8=0x%08x, r9=0x%08x, r10=0x%08x, r11=0x%08x\n", ad->r8, ad->r9, ad->r10, ad->r11);
  printf ("r12=0x%08x\n", ad->r12);
  printf ("\n");

  printf ("sp[0]=0x%08x, sp[1]=0x%08x, sp[2]=0x%08x, sp[3]=0x%08x\n", ad->stack [0], ad->stack [1], ad->stack [2], ad->stack [3]);
  printf ("sp[4]=0x%08x, sp[5]=0x%08x, sp[6]=0x%08x, sp[7]=0x%08x\n", ad->stack [4], ad->stack [5], ad->stack [6], ad->stack [7]);

  return 0;
}

static int monitorAbortClear (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  abortDat_t *ad = (abortDat_t *) &__abort_dat;

  memset (ad, 0, sizeof (* ad));

  return 0;
}

static int monitorAbortDirty (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  abortDat_t *ad = (abortDat_t *) &__abort_dat;

  ad->sigil = 0;

  return 0;
}

static int monitorAbortUndef (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  asm volatile (" .word 0x06000010" : /* no output */ : /* no inputs */ );

  return 0;
}

static int monitorAbortPabort (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  asm volatile (" ldr r0, =0x00080000" : /* no output */ : /* no inputs */ );
  asm volatile (" mov pc, r0" : /* no output */ : /* no inputs */ );

  return 0;
}

static int monitorAbortDabort (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  unsigned char c;
  volatile unsigned char *ptr = (unsigned char *) 0x40008000;

  c = *ptr;

  return 0;
}
#endif

#ifdef CFG_BEEP
//
//
//
static int monitorBeepOff (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  beepOff ();

  return 0;
}

static int monitorBeepOn (int argc __attribute__ ((unused)), portCHAR **argv)
{
  int  hz = atoi (argv [0]);

  if ((hz < 60) || (hz > 20000))
    printf ("Frequency must be between 60 and 20000 Hertz\n");
  else 
    beepOn (hz);

  return 0;
}

static int monitorBeepMHALL (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  beepMHALL ();

  return 0;
}

static int monitorBeepSMOTW (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  beepSMOTW ();

  return 0;
}
#endif

#ifdef CFG_AT24C1024
//
//
//
static int monitorEEIAddr (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;

  if (!getNumber (argv [0], &address))
    return 0;

  if (a24c1024SetAddress (address))
  {
    printf ("Error: address out of range\n");
    a24c1024SetAddress (0);
  }

  return 0;
}

static int monitorEEIRead (int argc, portCHAR **argv)
{
  unsigned int address;
  unsigned int length = 256;
  unsigned char buffer [64];
  unsigned int i;

  if (argc && !getNumber (argv [0], &length))
    return 0;

  for (address = a24c1024GetAddress (), i = 0; i < length; i += sizeof (buffer), address = (address + sizeof (buffer)) % A24C1024_SIZE)
  {
    unsigned int l;

    if (!(l = i % sizeof (buffer)))
      l = MIN (length, sizeof (buffer));

    if (a24c1024Read (buffer, l))
    {
      printf ("a24c1024Read() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
      return 0;
    }

    monitorDumpMemory (address, A24C1024_SIZE - 1, (unsigned int) buffer, l);
  }

  return 0;
}

static int monitorEEIReadAddr (int argc, portCHAR **argv)
{
  unsigned int address;
  
  if (!getNumber (argv [0], &address))
    return 0;

  if (a24c1024SetAddress (address))
  {
    printf ("Error: address out of range\n");
    return 0;
  }

  return monitorEEIRead (--argc, ++argv);
}

static int monitorEEIWriteCommon (int argc, portCHAR **argv, unsigned char *buffer, int bufferLength)
{
  int i;

  for (i = 0; i < argc; i++)
  {
    unsigned int n;

    if (i >= bufferLength)
    {
      printf ("Error: buffer too small for number of arguments\n");
      return -1;
    }

    if (!getNumber (argv [i], &n))
      return 0;

    if (n > 255)
    {
      printf ("Error: data must be 0x00..0xff (0..255)\n");
      return -1;
    }

    buffer [i] = n;
  }

  return 0;
}

//
//  Note the two reserved bytes at the beginning of the buffer.  These are
//  reserved for the address we're writing to in the EEPROM.  They're populated
//  by the a24c1024Write() routine.  This feel hackish, but unlike the read
//  routines, we can't send the address, then a repeated start bit to switch to
//  write.
//
static int monitorEEIWrite (int argc, portCHAR **argv)
{
  unsigned char buffer [18];

  if (monitorEEIWriteCommon (argc, argv, &buffer [2], sizeof (buffer) - 2))
    return 0;

  if (((a24c1024GetAddress () + (argc - 1)) ^ a24c1024GetAddress ()) & A24C1024_PAGESIZE)
    printf ("Error: write would cross a %d page boundary\n", A24C1024_PAGESIZE);
  else if (a24c1024Write (buffer, argc))
    printf ("a24c1024Write() returned %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));

  return 0;
}

static int monitorEEIWriteAddr (int argc, portCHAR **argv)
{
  unsigned int address;

  if (!getNumber (argv [0], &address))
    return 0;

  if (a24c1024SetAddress (address))
  {
    printf ("Error: address out of range\n");
    return 0;
  }

  return monitorEEIWrite (--argc, ++argv);
}

static int monitorEEIFillAddr (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;
  unsigned int length;
  unsigned int fillChar;

  if (!getNumber (argv [0], &address) || !getNumber (argv [1], &length) || !getNumber (argv [2], &fillChar))
    return 0;

  if (fillChar > 255)
    printf ("Error: fill value must be 0x00..0xff (0..255)\n");
  else if (a24c1024FillAddress (address, length, fillChar))
    printf ("a24c1024FillAddress() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));

  return 0;
}
#endif

#ifdef CFG_M25LC512
//
//  Why, yes, some of this code *is* nearly identical to the 'eei' commands...
//
static int m25lc512WriteGate = FALSE;

static int monitorEESuIPCheck (void)
{
#ifdef CFG_UIP
  if (uipIsRunning ())
  {
    printf ("Can't use 'ees' commands while uIP is running\n");
    return 1;
  }
#endif

  return 0;
}

static m25lc_e monitorEESWriteGate (void)
{
  if (m25lc512WriteGate)
  {
    m25lc_e r;

    if ((r = m25lc512WriteEnable ()) != M25LC_OK)
      printf ("m25lc512WriteEnable() returned error %d/%s\n", r, m25lc512Strerror (r));

    return r;
  }

  printf ("Write has not enabled with 'ees wren'\n");

  return M25LC_WP;
}

static int monitorEESAddr (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &address))
    return 0;

  if ((r = m25lc512SetAddress (address)) != M25LC_OK)
  {
    printf ("m25lc512SetAddress() returned error %d/%s\n", r, m25lc512Strerror (r));
    m25lc512SetAddress (0);
  }

  return 0;
}

static int monitorEESRead (int argc, portCHAR **argv)
{
  unsigned int address;
  unsigned int length = 256;
  unsigned char buffer [64];
  unsigned int i;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (argc && !getNumber (argv [0], &length))
    return 0;

  for (address = m25lc512GetAddress (), i = 0; i < length; i += sizeof (buffer), address = (address + sizeof (buffer)) % M25LC512_SIZE)
  {
    unsigned int l;

    if (!(l = i % sizeof (buffer)))
      l = MIN (length, sizeof (buffer));

    if ((r = m25lc512Read (buffer, l)) != M25LC_OK)
    {
      printf ("m25lc512Read() returned error %d/%s\n", r, m25lc512Strerror (r));
      return 0;
    }

    monitorDumpMemory (address, M25LC512_SIZE - 1, (unsigned int) buffer, l);
  }

  return 0;
}

static int monitorEESReadAddr (int argc, portCHAR **argv)
{
  unsigned int address;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &address))
    return 0;

  if ((r = m25lc512SetAddress (address)) != M25LC_OK)
  {
    printf ("m25lc512SetAddress() returned error %d/%s\n", r, m25lc512Strerror (r));
    return 0;
  }
  
  return monitorEESRead (--argc, ++argv);
}

static int monitorEESWriteCommon (int argc, portCHAR **argv, unsigned char *buffer, int bufferLength)
{
  int i;

  for (i = 0; i < argc; i++)
  {
    unsigned int n;

    if (i >= bufferLength)
    {
      printf ("Error: buffer too small for number of arguments\n");
      return -1;
    }

    if (!getNumber (argv [i], &n))
      return 0;

    if (n > 255)
    {
      printf ("Error: data must be 0x00..0xff (0..255)\n");
      return -1;
    }

    buffer [i] = n;
  }

  return 0;
}

static int monitorEESWrite (int argc, portCHAR **argv)
{
  unsigned char buffer [16];
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (monitorEESWriteCommon (argc, argv, buffer, sizeof (buffer)))
    return 0;

  if (monitorEESWriteGate () == M25LC_OK)
  {
    if (((m25lc512GetAddress () + (argc - 1)) ^ m25lc512GetAddress ()) & M25LC512_PAGESIZE)
      printf ("Error: write would cross a %d page boundary\n", M25LC512_PAGESIZE);
    else if ((r = m25lc512Write (buffer, argc)) != M25LC_OK)
      printf ("m25lc512Write() returned error %d/%s\n", r, m25lc512Strerror (r));
  }

  return 0;
}

static int monitorEESWriteAddr (int argc, portCHAR **argv)
{
  unsigned int address;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &address))
    return 0;

  if ((r = m25lc512SetAddress (address)) != M25LC_OK)
  {
    printf ("m25lc512SetAddress() returned error %d/%s\n", r, m25lc512Strerror (r));
    return 0;
  }

  return monitorEESWrite (--argc, ++argv);
}

static int monitorEESFillAddr (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;
  unsigned int length;
  unsigned int fillChar;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &address) || !getNumber (argv [1], &length) || !getNumber (argv [2], &fillChar))
    return 0;

  if (fillChar > 255)
    printf ("Error: fill value must be 0x00..0xff (0..255)\n");
  else if (monitorEESWriteGate () == M25LC_OK)
    if ((r = m25lc512FillAddress (address, length, fillChar)) != M25LC_OK)
      printf ("m25lc512FillAddress() returned error %d/%s\n", r, m25lc512Strerror (r));

  return 0;
}

static int monitorEESEraseChip (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (monitorEESWriteGate () == M25LC_OK)
    if ((r = m25lc512EraseChip ()) != M25LC_OK)
      printf ("m25lc512EraseChip() returned error %d/%s\n", r, m25lc512Strerror (r));

  return 0;
}

static int monitorEESEraseSector (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int sector;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &sector))
    return 0;

  if (monitorEESWriteGate () == M25LC_OK)
  {
    if ((r = m25lc512EraseSector (sector)) == M25LC_SECTORERR)
      printf ("Error:  sector out of range, must be 0..0x%x (0..%d)\n", M25LC512_LASTSECTOR, M25LC512_LASTSECTOR);
    else if (r != M25LC_OK)
      printf ("m25lc512EraseSector() returned error %d/%s\n", r, m25lc512Strerror (r));
  }

  return 0;
}

static int monitorEESErasePage (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int page;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &page))
    return 0;

  if (monitorEESWriteGate () == M25LC_OK)
  {
    if ((r = m25lc512ErasePage (page)) == M25LC_PAGEERR)
      printf ("Error:  page out of range, must be 0..0x%x (0..%d)\n", M25LC512_LASTPAGE, M25LC512_LASTPAGE);
    else if (r != M25LC_OK)
      printf ("m25lc512ErasePage() returned error %d/%s\n", r, m25lc512Strerror (r));
  }

  return 0;
}

static int monitorEESWriteEnable (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if ((r = m25lc512WriteEnable ()) != M25LC_OK)
    printf ("m25lc512WriteEnable() returned error %d/%s\n", r, m25lc512Strerror (r));
  else
    m25lc512WriteGate = TRUE;

  return 0;
}

static int monitorEESWriteDisable (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if ((r = m25lc512WriteDisable ()) != M25LC_OK)
    printf ("m25lc512WriteDisable() returned error %d/%s\n", r, m25lc512Strerror (r));
  else
    m25lc512WriteGate = FALSE;

  return 0;
}

static int monitorEESReadID (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  U8 id;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if ((r = m25lc512ReadID (&id)) != M25LC_OK)
    printf ("m25lc512ReadID() returned error %d/%s\n", r, m25lc512Strerror (r));
  else
    printf ("ID = 0x%02x (%d)\n", id, id);

  return 0;
}

static int monitorEESPowerDown (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if ((r = m25lc512DeepPowerDown ()) != M25LC_OK)
    printf ("m25lc512DeepPowerDown() returned error %d/%s\n", r, m25lc512Strerror (r));

  return 0;
}

static int monitorEESStatusRead (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  U8 status;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if ((r = m25lc512StatusRead (&status)) != M25LC_OK)
    printf ("m25lc512StatusRead() returned error %d/%s\n", r, m25lc512Strerror (r));
  else
    printf ("Status = 0x%02x (%d)\n", status, status);

  return 0;
}

static int monitorEESStatusWrite (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int status;
  m25lc_e r;

  if (monitorEESuIPCheck ())
    return 0;

  if (!getNumber (argv [0], &status))
    return 0;

  if (status > 255)
    printf ("Error: status must be 0x00..0xff (0..255)\n");
  else if (monitorEESWriteGate () == M25LC_OK)
    if ((r = m25lc512StatusWrite (status)) != M25LC_OK)
      printf ("m25lc512StatusWrite() returned error %d/%s\n", r, m25lc512Strerror (r));

  return 0;
}
#endif

#ifdef CFG_FIQ
//
//
//
static int monitorFIQClear (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  fiqClearCount ();

  return 0;
}

static int monitorFIQCount (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("FIQ counter = %u\n", fiqGetCount ());

  return 0;
}

static int monitorFIQOff (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("FIQ interrupts disabled, previous state was %s\n", fiqDisable () ? "enabled" : "disabled");

  return 0;
}

static int monitorFIQOn (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("FIQ interrupts enabled, previous state was %s\n", fiqEnable () ? "enabled" : "disabled");

  return 0;
}
#endif

#ifdef CFG_FATFS
//
//
//
static int monitorFileChmod (int argc __attribute__ ((unused)), portCHAR **argv)
{
  mode_t mode;

  if (!strcmp (argv [0], "-w"))
    mode = 0;
  else if (!strcmp (argv [0], "+w"))
    mode = S_IWUSR;
  else
  {
    printf ("mode must be +w (writable) or -w (not writable)\n");
    return 0;
  }

  if (chmod (argv [1], mode) == -1)
    printf ("chmod() failed, errno=%d/%s\n", errno, strerror (errno));

  return 0;
}

static int monitorFileConcat (int argc __attribute__ ((unused)), portCHAR **argv)
{
  int fd1st;
  int fd2nd;
  int l;
  char buffer [128];

  if ((fd1st = open (argv [0], O_RDONLY)) == -1)
  {
    printf ("Cannot open 1st file \"%s\", errno=%d/%s\n", argv [0], errno, strerror (errno));
    return 0;
  }

  if ((fd2nd = open (argv [1], O_WRONLY + O_CREAT + O_APPEND)) == -1)
  {
    printf ("Cannot open 2nd file \"%s\", errno=%d/%s\n", argv [1], errno, strerror (errno));
    return 0;
  }

  while ((l = read (fd1st, buffer, sizeof (buffer))))
  {
    if (write (fd2nd, buffer, l) != l)
    {
      printf ("write() returned error %d/%s (l=%d)\n", errno, strerror (errno), l);
      break;
    }
  }

  close (fd1st);
  close (fd2nd);

  return 0;
}

static int monitorFileCp (int argc __attribute__ ((unused)), portCHAR **argv)
{
  int fdIn;
  int fdOut;
  int l;
  char buffer [128];

  if ((fdIn = open (argv [0], O_RDONLY)) == -1)
  {
    printf ("Cannot open input file \"%s\", errno=%d/%s\n", argv [0], errno, strerror (errno));
    return 0;
  }

  if (argc == 1)
  {
    fdOut = fileno (stdout);
    fflush (stdout);
  }
  else if ((fdOut = open (argv [1], O_CREAT | O_TRUNC | O_WRONLY)) == -1)
  {
    printf ("Cannot open output file \"%s\", errno=%d/%s\n", argv [1], errno, strerror (errno));
    close (fdIn);
    return 0;
  }

  while ((l = read (fdIn, buffer, sizeof (buffer))))
  {
    if (write (fdOut, buffer, l) != l)
    {
      printf ("write() returned error %d/%s (l=%d)\n", errno, strerror (errno), l);
      break;
    }
  }

  close (fdIn);

  if (fdOut != fileno (stdout))
    close (fdOut);

  return 0;
}

static int monitorFileCpCon (int argc __attribute__ ((unused)), portCHAR **argv)
{
  int fdOut;
  char c;

  if ((fdOut = open (argv [0], O_CREAT | O_TRUNC | O_WRONLY)) == -1)
  {
    printf ("Cannot open output file \"%s\", errno=%d/%s\n", argv [0], errno, strerror (errno));
    return 0;
  }

  while (read (fileno (stdin), &c, sizeof (c)))
  {
    if (c == 0x04)
      break;

    if (c == '\r')
      c = '\n';

    if (write (fdOut, &c, sizeof (c)) != sizeof (c))
    {
      printf ("write() returned error %d/%s\n", errno, strerror (errno));
      break;
    }

    printf ("%c", c);
    fflush (stdout);
  }

  if (close (fdOut) == -1)
    printf ("close() returned error %d/%s\n", errno, strerror (errno));

  return 0;
}

static int monitorFileDf (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  U32 p2;
  FATFS *fs;
  char buffer [100];
  FRESULT res;
  int acc_size;
  int acc_files;
  int acc_dirs;

  if ((res = f_getfree ("", (U32 *) &p2, &fs)))
  { 
    f_printerror (res); 
    return 0;
  }

  printf ("FAT type = %u\nBytes/Cluster = %u\nNumber of FATs = %u\n"
      "Root DIR entries = %u\nSectors/FAT = %u\nNumber of clusters = %u\n"
      "FAT start (lba) = %u\nDIR start (lba,clustor) = %u\nData start (lba) = %u\n",
      fs->fs_type, fs->sects_clust * 512, fs->n_fats,
      fs->n_rootdir, fs->sects_fat, fs->max_clust - 2,
      fs->fatbase, fs->dirbase, fs->database
      );

  acc_size = acc_files = acc_dirs = 0;

  buffer [0] = '\0';

  if ((res = scan_files (buffer, &acc_size, &acc_files, &acc_dirs)))
  { 
    f_printerror (res); 
    return 0;
  }

  printf ("\n%u files, %u bytes.\n%u folders.\n"
      "%u bytes total disk space.\n%u bytes available\n",
      acc_files, acc_size, acc_dirs,
      (fs->max_clust - 2) * fs->sects_clust * 512, p2 * fs->sects_clust * 512
      );

  return 0;
}

static int monitorFileInit (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("RRC=%d\n", diskInitialize (0));

  return 0;
}

static int monitorFileLs (int argc, portCHAR **argv)
{
  DIR dir;
  FRESULT res;
  U32 size;
  U16 files;
  U16 dirs;
  FATFS *fs;
  char *path;

  path = argc ? argv [0] : "";

  if ((res = f_opendir (&dir, path)))
  { 
    f_printerror (res); 
    return 0;
  }

  for (size = files = dirs = 0;;)
  {
    if (((res = f_readdir (&dir, &fileInfo)) != FR_OK) || !fileInfo.fname [0]) 
      break;

    if (fileInfo.fattrib & AM_DIR) 
      dirs++;
    else 
    {
      files++; 
      size += fileInfo.fsize;
    }

    printf ("\n%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s",
        (fileInfo.fattrib & AM_DIR) ? 'D' : '-',
        (fileInfo.fattrib & AM_RDO) ? 'R' : '-',
        (fileInfo.fattrib & AM_HID) ? 'H' : '-',
        (fileInfo.fattrib & AM_SYS) ? 'S' : '-',
        (fileInfo.fattrib & AM_ARC) ? 'A' : '-',
        (fileInfo.fdate >> 9) + 1980, (fileInfo.fdate >> 5) & 15, fileInfo.fdate & 31,
        (fileInfo.ftime >> 11), (fileInfo.ftime >> 5) & 63,
        fileInfo.fsize, &(fileInfo.fname [0]));
  }

  printf ("\n%4u File(s),%10u bytes\n%4u Dir(s)", files, size, dirs);

  if (f_getfree (path, (U32 *) &size, &fs) == FR_OK)
    printf (", %10uK bytes free", size * fs->sects_clust / 2);

  printf ("\n");

  return 0;
}

static int monitorFileMkdir (int argc __attribute__ ((unused)), portCHAR **argv)
{
  f_printerror (f_mkdir (argv [0]));

  return 0;
}

static int monitorFileMkfs (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  f_printerror (f_mkfs (0, 0, 64));

  return 0;
}

static int monitorFileMount (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  f_printerror (f_mount (0, &fatfs));

  return 0;
}

static int monitorFileMv (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (rename (argv [0], argv [1]) == -1)
    printf ("rename failed, errno=%d/%s\n", errno, strerror (errno));

  return 0;
}

static int monitorFileRmCommon (char *path, int mode)
{
  FRESULT f;

  if ((f = f_stat (path, &fileInfo)) != FR_OK)
  {
    f_printerror (f);
    return 0;
  }

  if (mode == AM_DIR)
  {
    if (!(fileInfo.fattrib & AM_DIR))
      printf ("Not a directory\n");
    else
      f_printerror (f_unlink (path));
  }
  else
  {
    if (fileInfo.fattrib & AM_DIR)
      printf ("Not a regular file\n");
    else
      f_printerror (f_unlink (path));
  }

  return 0;
}

static int monitorFileRm (int argc __attribute__ ((unused)), portCHAR **argv)
{
  return monitorFileRmCommon (argv [0], 0);
}

static int monitorFileRmdir (int argc __attribute__ ((unused)), portCHAR **argv)
{
  return monitorFileRmCommon (argv [0], AM_DIR);
}

static int monitorFileSync (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  sync ();

  return 0;
}

int monitorTimevalSubtract (struct timeval *result, struct timeval *x, struct timeval *y);
int monitorTimevalSubtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  if (x->tv_usec < y->tv_usec)
  {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }

  if (x->tv_usec - y->tv_usec > 1000000)
  {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  return x->tv_sec < y->tv_sec;
}

typedef enum
{
  MODE_NORMAL = 0,
  MODE_NOINTS,
  MODE_SUSPENDALL,
  MODE_HIGH
}
mode_e;

static int monitorFileThruPut (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int i;
  mode_e mode;
  int fileSizes [] = { 1*1024, 8*1024, 16*1024, 64*1024, 128*1024, 512*1024, 1024*1024, 2048*1024 };

  if (!strcmp (argv [0], "normal"))
    mode = MODE_NORMAL;
  else if (!strcmp (argv [0], "noints"))
    mode = MODE_NOINTS;
  else if (!strcmp (argv [0], "suspendall"))
    mode = MODE_SUSPENDALL;
  else if (!strcmp (argv [0], "high"))
    mode = MODE_HIGH;
  else
  {
    printf ("arguments must be 'normal', 'noints', 'suspendall', or 'high'\n");
    return 0;
  }

  printf ("%lu tests, with ", arrsizeof (fileSizes));

  switch (mode)
  {
    case MODE_NORMAL     : printf ("interrupts enabled, no tasks suspended, default priority\n\n"); break;
    case MODE_NOINTS     : printf ("interrupts disabled (no tasking)\n\n"); break;
    case MODE_SUSPENDALL : printf ("interrupts enabled, all tasks suspended\n\n"); break;
    case MODE_HIGH       : printf ("interrupts enabled, this task promoted to highest priority\n\n");
  }

  for (i = 0; i < arrsizeof (fileSizes); i++)
  {
    int j;

    for (j = 0; j < 2; j++)
    {
      int fd;
      int k;
      char buffer [512];
      struct timeval tv_start;
      struct timeval tv_end;
      struct timeval tv_diff;
      unsigned long ticks10ms;
      unsigned portBASE_TYPE taskPriority = 0;

      if ((fd = open ("testfile.bin", !j ? (O_TRUNC | O_CREAT | O_WRONLY) : (O_RDONLY))) == -1)
      {
        printf ("Cannot open \"testfile.bin\", errno=%d/%s\n", errno, strerror (errno));
        return 0;
      }

      if (!j)
        memset (buffer, 0xe5, sizeof (buffer));


      switch (mode)
      {
        case MODE_NORMAL     : break;
        case MODE_NOINTS     : portENTER_CRITICAL (); break;
        case MODE_SUSPENDALL : vTaskSuspendAll (); break;
        case MODE_HIGH       : taskPriority = uxTaskPriorityGet (NULL); vTaskPrioritySet (NULL, (configMAX_PRIORITIES - 1)); break;
      }

      gettimeofday (&tv_start, NULL);

      if (!j)
      {
        for (k = fileSizes [i]; k > 0; k -= sizeof (buffer))
        {
          if (write (fd, buffer, sizeof (buffer)) != sizeof (buffer))
          {
            printf ("write() failed, errno=%d/%s\n", errno, strerror (errno));
            close (fd);
            return 0;
          }
        }
      }
      else
      {
        for (k = fileSizes [i]; k > 0; k -= sizeof (buffer))
        {
          if (read (fd, buffer, sizeof (buffer)) != sizeof (buffer))
          {
            printf ("read() failed, errno=%d/%s\n", errno, strerror (errno));
            close (fd);
            return 0;
          }
        }
      }

      gettimeofday (&tv_end, NULL);

      switch (mode)
      {
        case MODE_NORMAL     : break;
        case MODE_NOINTS     : portEXIT_CRITICAL (); break;
        case MODE_SUSPENDALL : xTaskResumeAll (); break;
        case MODE_HIGH       : vTaskPrioritySet (NULL, taskPriority); break;
      }

      if (close (fd) == -1)
      {
        printf ("close() failed, errno=%d/%s\n", errno, strerror (errno));
        return 0;
      }

      {
        struct timeval tempStart = tv_start;
        struct timeval tempEnd = tv_end;

        monitorTimevalSubtract (&tv_diff, &tv_end, &tv_start);
        ticks10ms = (tv_diff.tv_sec * 100) + (tv_diff.tv_usec / 10000);

        printf ("%5s: %9u bytes, %7lu milliseconds (%4ld seconds), %7lu bytes/sec\n", 
            !j ? "Write" : "Read", 
            fileSizes [i], 
            ticks10ms * 10,
            ticks10ms / 100,
            (fileSizes [i] * 100) / ticks10ms);

        if (ticks10ms > 10000)
        {
          printf ("tv_start = %ld, %ld\n", tempStart.tv_sec, tempStart.tv_usec);
          printf ("tv_end   = %ld, %ld\n", tempEnd.tv_sec, tempEnd.tv_usec);
          printf ("tv_diff  = %ld, %ld\n", tv_diff.tv_sec, tv_diff.tv_usec);
        }
      }

      if (j && unlink ("testfile.bin") == -1)
      {
        printf ("unlink() failed, errno=%d/%s\n", errno, strerror (errno));
        return 0;
      }
    }
  }

  printf ("\nDone\n");

  return 0;
}
#endif

#ifdef CFG_GPS
//
//
//
static int monitorGPSShow (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  gpsData_t gpsData;

  if (gpsCopyData (&gpsData))
  {
    printf ("Valid ...... : %d\n", gpsData.valid);
    printf ("Date ....... : %04d/%02d/%02d\n", gpsData.utcYear, gpsData.utcMonth, gpsData.utcDay);
    printf ("Time ....... : %02d:%02d:%02d\n", gpsData.utcHours, gpsData.utcMinutes, gpsData.utcSeconds);
    printf ("Speed ...... : %f\n", gpsData.groundSpeed);
    printf ("Heading .... : %f\n", gpsData.trueCourse);
    printf ("Altitude ... : %f\n", gpsData.height);
    printf ("Latitude ... : %f\n", gpsData.latitude);
    printf ("Longitude .. : %f\n", gpsData.longitude);
    printf ("Restarts ... : %d\n", gpsData.restarts);
    printf ("Sizeof () .. : %ld\n", sizeof (gpsData));
  }
  else
    printf ("Unable to get GPS data\n");

  return 0;
}

static int monitorGPSStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (taskHandles [TASKHANDLE_GPS])
    printf ("GPS task is already running.  Use 'gps stop' to stop it\n");
  else
    gpsTaskStart ();

  return 0;
}

static int monitorGPSStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!gpsTaskStop ())
    printf ("GPS task is not yet running.  Use 'gps start' to start it\n");

  return 0;
}
#endif

#ifdef CFG_I2C
//
//
//
int monitorI2CRead (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;
  unsigned int numBytes;
  unsigned char buffer [16];
  int r;

  if (!getNumber (argv [0], &address))
    return 0;
  if (!getNumber (argv [1], &numBytes))
    return 0;

  if (address > 255)
  {
    printf ("Error: address must be 0x00..0xff (0..255)\n");
    return 0;
  }

  if ((numBytes < 1) || (numBytes > sizeof (buffer)))
  {
    printf ("Error: number of bytes must be 1..%ld\n", sizeof (buffer));
    return 0;
  }

  r = i2cReadBuffer (address, buffer, numBytes);

  printf ("i2cReadBuffer() returned %d/%s (%s)\n\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()), r ? "error" : "no error");

  monitorDumpMemory (0, 0xffffffff, (unsigned int) buffer, (int) sizeof (buffer));

  return 0;
}

int monitorI2CWrite (int argc, portCHAR **argv)
{
  unsigned int address;
  unsigned char buffer [16];
  int i;

  if (!getNumber (argv [0], &address))
    return 0;

  if (address > 255)
  {
    printf ("Error: address must be 0x00..0xff (0..255)\n");
    return 0;
  }

  for (i = 0; i < argc - 1; i++)
  {
    unsigned int n;

    if (!getNumber (argv [i + 1], &n))
      return 0;

    if (n > 255)
    {
      printf ("Error: data must be 0x00..0xff (0..255)\n");
      return 0;
    }

    buffer [i] = n;
  }

  i = i2cWriteBuffer (address, buffer, argc - 1);

  printf ("i2cWriteBuffer() returned %d/%s (%s)\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()), i ? "error" : "no error");

  return 0;
}

int monitorI2CWriteRead (int argc, portCHAR **argv)
{
  unsigned int address;
  unsigned int bytesToWrite;
  unsigned int bytesToRead;
  unsigned char buffer [16];
  unsigned int i;

  if (!getNumber (argv [0], &address))
    return 0;

  if (address > 255)
  {
    printf ("Error: address must be 0x00..0xff (0..255)\n");
    return 0;
  }

  for (bytesToWrite = argc - 2, i = 0; i < bytesToWrite; i++)
  {
    unsigned int n;

    if (!getNumber (argv [i + 1], &n))
      return 0;

    if (n > 255)
    {
      printf ("Error: data must be 0x00..0xff (0..255)\n");
      return 0;
    }

    buffer [i] = n;
  }

  if (!getNumber (argv [argc - 1], &bytesToRead))
    return 0;

  if ((bytesToRead < 1) || (bytesToRead > sizeof (buffer)))
  {
    printf ("Error: number of bytes must be 1..%ld\n", sizeof (buffer));
    return 0;
  }

  i2cWriteReadBuffer (address, buffer, bytesToWrite, bytesToRead);

  printf ("i2cWriteReadBuffer() returned %d/%s\n\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));

  monitorDumpMemory (0, 0xffffffff, (unsigned int) buffer, (int) sizeof (buffer));

  return 0;
}

int monitorI2CDump (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  i2cDump ();

  return 0;
}

int monitorI2CErrno (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("i2cErrno=%d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));

  return 0;
}
#endif

#ifdef CFG_IAP
//
//  Note that although the iapXXX calls can deal with multiple sectors, we only ever
//  deal with one at a time.  This simplifies the user interface.
//
static int monitorIAPFSS (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int ss;

  if ((ss = iapFindSafeSector ()) == -1)
    printf ("Can't find a safe sector!  All of flash in use?\n");
  else
    printf ("Sector number %d is safe for IAP operations (won't overwrite any code)\n", ss);

  return 0;
}

static int monitorIAPSTOA (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int sector;
  unsigned long address;
  int sectorSize;

  if (!getNumber (argv [0], &sector))
    return 0;

  if (iapSectorToAddress (sector, &address, &sectorSize) == -1)
    printf ("Sector number out of range\n");
  else
    printf ("Sector %d occupies address 0x%08lx for 0x%04x bytes\n", sector, address, sectorSize);

  return 0;
}

static int monitorIAPFill (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int sector;
  unsigned int byte;

  if (!getNumber (argv [0], &sector))
    return 0;
  if (!getNumber (argv [1], &byte))
    return 0;

  if (!iapIsSafeSector ((int) sector))
  {
    printf ("Sector number out of range or not in safe region\n");
    return 0;
  }

  if (byte > 255)
  {
    printf ("Fill character must be 0x00..0xff (0..255)\n");
    return 0;
  }

  if (iapFillSectors (sector, sector, byte) == -1)
    printf ("iapFillSectors returned error %d/%s\n", iapGetErrno (), iapStrerror (iapGetErrno ()));
  else
    printf ("Sector %d filled with 0x%02x\n", sector, byte);

  return 0;
}

static int monitorIAPErase (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int sector;

  if (!getNumber (argv [0], &sector))
    return 0;

  if (!iapIsSafeSector ((int) sector))
  {
    printf ("Sector number out of range or not in safe region\n");
    return 0;
  }

  if (iapEraseSectors (sector, sector) == -1)
    printf ("iapEraseSectors returned error %d/%s\n", iapGetErrno (), iapStrerror (iapGetErrno ()));
  else
    printf ("Sector %d erased\n", sector);

  return 0;
}

static int monitorIAPBlank (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int sector;

  if (!getNumber (argv [0], &sector))
    return 0;

  if (!iapIsValidSector ((int) sector))
  {
    printf ("Sector number out of range\n");
    return 0;
  }

  switch (iapBlankCheckSectors (sector, sector))
  {
    case 0 :
      printf ("Sector %d is blank\n", sector);
      break;

    case 1 :
      printf ("Sector %d is not blank\n", sector);
      break;

    default :
      printf ("iapBlankCheckSector returned error %d/%s\n", iapGetErrno (), iapStrerror (iapGetErrno ()));
      break;
  }

  return 0;
}

static int monitorIAPID (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("IAP ID=0x%08x\n", iapReadPartID ());

  return 0;
}

static int monitorIAPVer (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("IAP ID=0x%08x\n", iapReadBootCodeVersion ());

  return 0;
}

static int monitorIAPISP (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  iapISP ();

  return 0;
}
#endif

#ifdef CFG_KBD
//
//
//
int monitorKbdGet (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int key;

  if (!taskHandles [TASKHANDLE_KBD])
    printf ("Keyboard task is not yet running.  Use 'kbd start' to start it\n");
  else if ((key = kbdGetKey ()))
    printf ("Got %c from keyboard\n", key);
  else
    printf ("No keys waiting\n");

  return 0;
}

int monitorKbdStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (taskHandles [TASKHANDLE_KBD])
    printf ("Keyboard task is already running.  Use 'kbd stop' to stop it\n");
  else
  {
#ifdef CFG_LCD
    lcdTaskStop ();
#endif
    kbdTaskStart ();
  }

  return 0;
}

int monitorKbdStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!kbdTaskStop ())
    printf ("Keyboard task is not yet running.  Use 'kbd start' to start it\n");

  return 0;
}

#endif

#ifdef CFG_LCD
//
//  DANGER!  Do not change the task priority to be higher than that of the
//  monitor.  If the LCD is not present, it will block any lower priority tasks
//  while waiting for the LCD status line to go active.  This means you will
//  not be able to issue 'lcd stop' from the monitor command line.
//
int monitorLCDStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (taskHandles [TASKHANDLE_LCD])
    printf ("LCD task is already running.  Use 'lcd stop' to stop it\n");
  else
  {
#ifdef CFG_KBD
    kbdTaskStop ();
#endif
    lcdTaskStart ();
  }

  return 0;
}

int monitorLCDStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!lcdTaskStop ())
    printf ("LCD task is not yet running.  Use 'lcd start' to start it\n");

  return 0;
}

static int monitorLCDClear (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!taskHandles [TASKHANDLE_LCD])
    printf ("LCD task is not yet running.  Use 'lcd start' to start it\n");
  else
    lcdWriteText ("\f");

  return 0;
}

static int monitorLCDGotoXY (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (!taskHandles [TASKHANDLE_LCD])
    printf ("LCD task is not yet running.  Use 'lcd start' to start it\n");
  else
  {
    static char buffer [5];

    if (!isDecimalString (argv [0]) || !isDecimalString (argv [1]))
    {
      printf ("Malformed number.  Must be decimal number.\n");
      return 0;
    }

    buffer [0] = 0x1b;
    buffer [1] = '=';
    buffer [2] = atoi (argv [0]);
    buffer [3] = atoi (argv [1]);
    buffer [4] = '\0';

    lcdWriteText (buffer);
  }

  return 0;
}

static int monitorLCDMessage (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (!taskHandles [TASKHANDLE_LCD])
    printf ("LCD task is not yet running.  Use 'lcd start' to start it\n");
  else
  {
    char *s = argv [0];

    if ((s [0] == '"') && (s [strlen (s) - 1] == '"'))
    {
      s++;
      s [strlen (s) - 1] = '\0';
    }

    lcdWriteText (s);
  }

  return 0;
}
#endif

#ifdef CFG_LED
//
//
//
int monitorLEDStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (taskHandles [TASKHANDLE_LED])
    printf ("LED task is already running.  Use 'led stop' to stop it\n");
  else
    ledsTaskStart ();

  return 0;
}

int monitorLEDStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!ledsTaskStop ())
    printf ("LED task is not yet running.  Use 'led start' to start it\n");

  return 0;
}
#endif

#ifdef CFG_LM75
//
//
//
int monitorLM75Init (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (lm75Init ())
    printf ("lm75Init() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));

  return 0;
}

int monitorLM75Mode (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int mode;

  if (!getNumber (argv [0], &mode))
    return 0;

  if (mode > 1)
    printf ("Error: mode must be 0 or 1\n");
  else
  {
    lm75SetMode (mode);

    if (!mode)
      printf ("lm75 using I2C repeated start for write then read\n");
    else
      printf ("lm75 using I2C stop then start for read\n");
  }

  return 0;
}

int monitorLM75Addr (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int address;

  if (!getNumber (argv [0], &address))
    return 0;

  if (address > 0xff)
    printf ("Error: address must be 0x00..0xff (0..255)\n");
  else
    lm75SetAddress (address);

  return 0;
}

int monitorLM75ReRead (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int value;

  if (lm75ReRead (&value))
    printf ("lm75Read() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
  else
    printf ("value=0x%x\n", value);

  return 0;
}

int monitorLM75Temp (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int temp;

  if (lm75TemperatureRead (&temp))
    printf ("lm75TemperatureRead() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
  else
    printf ("temp=%d.%dC\n", temp >> 1, (temp & 0x01) ? 5 : 0);

  return 0;
}

int monitorLM75Config (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int config;

  if (lm75ConfigRead (&config))
    printf ("lm75ConfigRead() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
  else
    printf ("config=0x%02x\n", config);

  return 0;
}

int monitorLM75Thyst (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int thyst;

  if (!argc)
  {
    if (lm75THYSTRead (&thyst))
      printf ("lm75THYSTRead() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
    else
      printf ("THYST=%d.%dC\n", thyst / 2, (thyst & 0x01) ? 5 : 0);
  }
  else
  {
    if (!getNumber (argv [0], (unsigned int *) &thyst))
      return 0;

    if ((thyst < -55) || (thyst > 125))
      printf ("Error: THYST range is -55C to +125C\n");
    else if (lm75THYSTWrite (thyst << 1))
      printf ("lm75THYSTWrite() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
  }
  return 0;
}

int monitorLM75Tos (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int tos;

  if (!argc)
  {
    if (lm75TOSTRead (&tos))
      printf ("lm75TOSRead() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
    else
      printf ("TOS=%d.%dC\n", tos / 2, (tos & 0x01) ? 5 : 0);
  }
  else 
  {
    if (!getNumber (argv [0], (unsigned int *) &tos))
      return 0;
    
    if ((tos < -55) || (tos > 125))
      printf ("Error: TOS range is -55C to +125C\n");
    else if (lm75TOSWrite (tos << 1))
      printf ("lm75TOSWrite() returned error %d/%s\n", i2cGetErrno (), i2cStrerror (i2cGetErrno ()));
  }

  return 0;
}
#endif

#ifdef CFG_MEM
//
//
//
static int monitorMemTask (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
#if configUSE_TRACE_FACILITY == 1
  signed portCHAR buffer [TASKHANDLE_LAST * 42];
  int bytesFree;
  int bytesUsed;
  int blocksFree;
  signed portCHAR *s = buffer;

  vTaskList (buffer);
  vPortUsedMem (&bytesFree, &bytesUsed, &blocksFree);

  while ((s = (signed portCHAR *) index ((char *) s, '\r')))
    *s = ' ';

  //
  //  Stack is the minimum amount of stack there has been available at any time since the task started executing.
  //
  printf ("Task\t\tState\tPrty\tStack\tTask#\n");
  printf ("%s\nHeap size=%ld, used=%d, free=%d (%d blocks)\n", buffer, configTOTAL_HEAP_SIZE, bytesUsed, bytesFree, blocksFree);
#else
  printf ("Not implemented (requires configUSE_TRACE_FACILITY in FreeRTOSConfig.h)\n");
#endif

  return 0;
}

typedef struct memMap_s
{
  char *desc;
  int m;
  unsigned int start;
  unsigned int end;
}
memMap_t;

typedef union 
{
  void *v;
  unsigned int i;
}
sbrkConv_t;

static inline unsigned __get_cpsr (void)
{
  unsigned long retval;

  asm volatile (" mrs  %0, cpsr" : "=r" (retval) : /* no inputs */  );

  return retval;
}

static int monitorMemMap (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  unsigned int i;
  sbrkConv_t sbrkConv;
  static memMap_t memMap [] = 
  {
    { ".startup .....", 0, (unsigned int) &__start_of_startup__, (unsigned int) &__end_of_startup__ },
    { ".text ........", 0, (unsigned int) &__start_of_text__,    (unsigned int) &__end_of_text__ },
    { ".code ........", 0, (unsigned int) &__start_of_prog__,    (unsigned int) &__end_of_prog__ },
    { ".rodata ......", 0, (unsigned int) &__start_of_rodata__,  (unsigned int) &__end_of_rodata__ },
    { ".data ........", 0, (unsigned int) &__data_beg__,         (unsigned int) &__data_end__ },
    { ".bss .........", 0, (unsigned int) &__bss_beg__,          (unsigned int) &__bss_end__ },
    { "heap .........", 1, (unsigned int) &__heap_beg,           (unsigned int) &__heap_end },
    { "heap range ...", 1, (unsigned int) &__heap_beg,           (unsigned int) &__heap_max },
//  { "SYS stack ....", 1, (unsigned int) &__stack_beg_sys,      (unsigned int) &__stack_end_sys }, // Not relevant to FreeRTOS
    { "SVC stack ....", 1, (unsigned int) &__stack_beg_svc,      (unsigned int) &__stack_end_svc },
    { "IRQ stack ....", 1, (unsigned int) &__stack_beg_irq,      (unsigned int) &__stack_end_irq },
    { "FIQ stack ....", 1, (unsigned int) &__stack_beg_fiq,      (unsigned int) &__stack_end_fiq },
    { "abort stack ..", 1, (unsigned int) &__stack_beg_abt,      (unsigned int) &__stack_end_abt },
    { "undef stack ..", 1, (unsigned int) &__stack_beg_und,      (unsigned int) &__stack_end_und },
  };

  sbrkConv.v = sbrk (0);
  __heap_end = sbrkConv.i;
  __stack_end_sys = sbrkConv.i;

  printf ("Section        Start      End        Length\n");
  printf ("-------------------------------------------\n");

  for (i = 0; i < arrsizeof (memMap); i++)
  {
    if (!memMap [i].m)
      printf ("%s 0x%08x 0x%08x 0x%x\n", memMap [i].desc, memMap [i].start, memMap [i].end, abs (memMap [i].end - memMap [i].start));
    else
      printf ("%s 0x%08x 0x%08x 0x%x\n", memMap [i].desc, *(unsigned int *) memMap [i].start, *(unsigned int *) memMap [i].end, abs (*(unsigned int *) memMap [i].end - *(unsigned int *) memMap [i].start));
  }

#if 0
  printf ("\nProcessor mode ");

  switch ((i = __get_cpsr ()) & 0x1f)
  {
    case 0x10 : printf ("User"); break;
    case 0x11 : printf ("FIQ"); break;
    case 0x12 : printf ("IRQ"); break;
    case 0x13 : printf ("Supervisor"); break;
    case 0x17 : printf ("Abort"); break;
    case 0x1b : printf ("Undefined"); break;
    case 0x1f : printf ("System"); break;
    default   : printf ("Unknown");
  }

  printf (", IRQ %s", (i & 0x80) ? "disabled" : "enabled");
  printf (", FIQ %s", (i & 0x40) ? "disabled" : "enabled");
  printf (", mode %s\n", (i & 0x20) ? "THUMB" : "ARM");
#endif

  printf ("\n");
  malloc_stats ();

  return 0;
}

typedef struct memorySlots_s
{
  unsigned char *p;
  unsigned int size;
}
memorySlots_t;

memorySlots_t memorySlots [8];

static int monitorMemAlloc (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int size;
  unsigned int slot;
  unsigned char *p;

  if (!getNumber (argv [0], &slot) || !getNumber (argv [0], &size))
    return 0;

  if (slot >= arrsizeof (memorySlots))
  {
    printf ("slot must be 0..%lu\n", arrsizeof (memorySlots) - 1);
    return 0;
  }

  if (memorySlots [slot].p)
    printf ("Slot %d in use, free it first, or use realloc\n", slot);
  else if (!(p = malloc (size)))
    printf ("malloc() failed, error %d/%s\n", errno, strerror (errno));
  else
  {
    memorySlots [slot].p = p;
    memorySlots [slot].size = size;
  }

  return 0;
}

static int monitorMemRealloc (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int size;
  unsigned int slot;
  unsigned char *p;

  if (!getNumber (argv [0], &slot) || !getNumber (argv [0], &size))
    return 0;

  if (slot >= arrsizeof (memorySlots))
  {
    printf ("slot must be 0..%lu\n", arrsizeof (memorySlots) - 1);
    return 0;
  }

  if (!(p = realloc (memorySlots [slot].p, size)))
    printf ("realloc() failed, error %d/%s\n", errno, strerror (errno));
  else
  {
    memorySlots [slot].p = p;
    memorySlots [slot].size = size;
  }

  return 0;
}

static int monitorMemFree (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int slot;

  if (!getNumber (argv [0], &slot))
    return 0;

  if (slot >= arrsizeof (memorySlots))
  {
    printf ("slot must be 0..%lu\n", arrsizeof (memorySlots) - 1);
    return 0;
  }

  if (!memorySlots [slot].p)
    printf ("Can't free it, slot %d not in use\n", slot);
  else
  {
    free (memorySlots [slot].p);
    memorySlots [slot].p = NULL;
    memorySlots [slot].size = 0;
  }

  return 0;
}

static int monitorMemList (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  unsigned int i;

  printf ("Slot  Address     Size\n");
  printf ("----------------------\n");

  for (i = 0; i < arrsizeof (memorySlots); i++)
    printf ("%4d  0x%08x  %d\n", i, (unsigned int) memorySlots [i].p, memorySlots [i].size);

  return 0;
}
#endif

#ifdef CFG_MISC
//
//
//
static int monitorMiscPorts (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  unsigned int gpio0_fiopin;
  unsigned int gpio0_fiodir;
  unsigned int gpio0_fiomask;
  unsigned int gpio1_fiopin;
  unsigned int gpio1_fiodir;
  unsigned int gpio1_fiomask;

  portENTER_CRITICAL ();

  gpio0_fiopin  = GPIO0_FIOPIN;
  gpio0_fiodir  = GPIO0_FIODIR;
  gpio0_fiomask = GPIO0_FIOMASK;
  gpio1_fiopin  = GPIO1_FIOPIN;
  gpio1_fiodir  = GPIO1_FIODIR;
  gpio1_fiomask = GPIO1_FIOMASK;

  portEXIT_CRITICAL ();

  printf ("GPIO0_FIOPIN  = 0x%08x\n", gpio0_fiopin);
  printf ("GPIO0_FIODIR  = 0x%08x\n", gpio0_fiodir);
  printf ("GPIO0_FIOMASK = 0x%08x\n", gpio0_fiomask);
  printf ("GPIO1_FIOPIN  = 0x%08x\n", gpio1_fiopin);
  printf ("GPIO1_FIODIR  = 0x%08x\n", gpio1_fiodir);
  printf ("GPIO1_FIOMASK = 0x%08x\n", gpio1_fiomask);

  return 0;
}

static int monitorMiscSizeof (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("sizeof (char)      = %lu\n", sizeof (char));
  printf ("sizeof (short)     = %lu\n", sizeof (short));
  printf ("sizeof (int)       = %lu\n", sizeof (int));
  printf ("sizeof (long)      = %lu\n", sizeof (long));
  printf ("sizeof (long long) = %lu\n", sizeof (long long));
  printf ("sizeof (float)     = %lu\n", sizeof (float));
  printf ("sizeof (double)    = %lu\n", sizeof (double));
  printf ("sizeof (void *)    = %lu\n", sizeof (void *));

  return 0;
}
#endif

#ifdef CFG_PWM
//
//
//
static int monitorPWMDuty (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (pwmDutyCycle (atoi (argv [0])))
    printf ("PWM duty cycle must be between 0 and 100 percent\n");

  return 0;
}

static int monitorPWMFrequency (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (pwmFrequency (atoi (argv [0])))
    printf ("PWM duty cycle must be between 1 and %ldHz\n", configCPU_CLOCK_HZ);

  return 0;
}

static int monitorPWMWidth (int argc __attribute__ ((unused)), portCHAR **argv)
{
  if (pwmPulseWidthInMicroseconds (atoi (argv [0])))
    printf ("PWM duty cycle must be between 0 and %d\n", PWM_MR0);

  return 0;
}
#endif

#ifdef CFG_RTC
//
//
//
static int monitorRTCGet (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  time_t now;
  char buffer [32];

  now = time (NULL);
  ctime_r (&now, buffer);
  printf ("%s", buffer);

  return 0;
}

//
//  This works because atoi() stops on a non-digit.  We already know the 
//  string values are digits, because of isGoodString().
//
static int tmSetElement (int *element, int minValue, int maxValue, int adjustValue, char *s)
{
  int v;
  
  if (((v = atoi (s)) < minValue) || (v > maxValue))
  {
    printf ("Illegal value encountered in parameter\n");
    return FALSE;
  }

  *element = (v + adjustValue);

  return TRUE;
}

static int isGoodString (char *s, char *fmt)
{
  unsigned int i;
  unsigned int l;

  if ((l = strlen (s)) != strlen (fmt))
    return FALSE;

  for (i = 0; i < l; i++, s++, fmt++)
  {
    if (*fmt == 'N')
    {
      if (!isdigit (*s))
        return FALSE;
    }
    else if (*fmt != *s)
      return FALSE;
  }

  return TRUE;
}

static int monitorRTCSet (int argc, portCHAR **argv)
{
  if ((argc == 1) && !strcmp (argv [0], "gps"))
  {
#ifdef CFG_GPS
    gpsData_t gpsData;

    if (gpsCopyData (&gpsData))
    {
      if (gpsData.valid)
      {
        struct tm tm;

        tm.tm_sec   = gpsData.utcSeconds;
        tm.tm_min   = gpsData.utcMinutes;
        tm.tm_hour  = gpsData.utcHours;
        tm.tm_mday  = gpsData.utcDay;
        tm.tm_mon   = gpsData.utcMonth - 1;
        tm.tm_year  = gpsData.utcYear - 1900;
        tm.tm_wday  = 0;
        tm.tm_yday  = 0;
        tm.tm_isdst = 0;

        rtcSetEpochSeconds (mktime (&tm));

        printf ("Date/time set from GPS\n");
      }
      else
        printf ("GPS data not valid\n");
    }
    else
      printf ("Unable to get GPS data\n");
#else
    printf ("Can't set time from GPS, as GPS support is disabled\n");
#endif
  }
  else if (isGoodString (argv [0], "NNNN/NN/NN") && isGoodString (argv [1], "NN:NN:NN"))
  {
    struct tm tm;

    if (!tmSetElement (&tm.tm_year, 1900, 2038, -1900, &argv [0][0]))
      return 0;
    if (!tmSetElement (&tm.tm_mon, 1, 12, -1, &argv [0][5]))
      return 0;
    if (!tmSetElement (&tm.tm_mday, 1, 31, 0, &argv [0][8]))
      return 0;
    if (!tmSetElement (&tm.tm_hour, 0, 23, 0, &argv [1][0]))
      return 0;
    if (!tmSetElement (&tm.tm_min, 0, 59, 0, &argv [1][3]))
      return 0;
    if (!tmSetElement (&tm.tm_sec, 0, 59, 0, &argv [1][6]))
      return 0;

    tm.tm_wday  = 0;
    tm.tm_yday  = 0;
    tm.tm_isdst = 0;

    rtcSetEpochSeconds (mktime (&tm));
  }
  else
    printf ("Parameter(s) either not 'gps' or not 'YYYYY/MM/DD HH:MM:SS'\n");

  return 0;
}

static int monitorRTCAlarm (int argc, portCHAR **argv)
{
  if (!argc)
  {
    time_t a;

    if ((a = rtcGetAlarmEpochSeconds ()))
    {
      char buffer [32];
      ctime_r (&a, buffer);
      printf ("%s", buffer);
    }
    else
      printf ("alarm not enabled\n");
  }
  else if (argc == 1)
  {
    if (!strcmp (argv [0], "off"))
      rtcSetAlarm (NULL);
    else
      printf ("'rtc alarm' requires either 'off' or 'YYYY/MM/DD HH:MM:SS' as arguments\n");
  }
  else if (isGoodString (argv [0], "NNNN/NN/NN") && isGoodString (argv [1], "NN:NN:NN"))
  {
    struct tm tm;

    if (!tmSetElement (&tm.tm_year, 1900, 2038, -1900, &argv [0][0]))
      return 0;
    if (!tmSetElement (&tm.tm_mon, 1, 12, -1, &argv [0][5]))
      return 0;
    if (!tmSetElement (&tm.tm_mday, 1, 31, 0, &argv [0][8]))
      return 0;
    if (!tmSetElement (&tm.tm_hour, 0, 23, 0, &argv [1][0]))
      return 0;
    if (!tmSetElement (&tm.tm_min, 0, 59, 0, &argv [1][3]))
      return 0;
    if (!tmSetElement (&tm.tm_sec, 0, 59, 0, &argv [1][6]))
      return 0;

    if (rtcSetAlarm (&tm))
      printf ("Cannot set the alarm for the past\n");
  }
  else
    printf ("Parameter(s) either not 'off' or not 'YYYYY/MM/DD HH:MM:SS'\n");

  return 0;
}

static int monitorRTCPeriodic (int argc, portCHAR **argv)
{
  if (!argc)
    printf ("Current periodic alarm state is %s\n", rtcPeriodicAlarm (-1) ? "on" : "off");
  else if (!strcmp (argv [0], "off"))
    rtcPeriodicAlarm (0);
  else if (!strcmp (argv [0], "on"))
    rtcPeriodicAlarm (1);
  else
    printf ("Error: if argument present, must be 'off' or 'on'\n");

  return 0;
}
#endif

#ifdef CFG_SENSORS
//
//
//
static int monitorSensorsShow (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!taskHandles [TASKHANDLE_SENSORS])
    printf ("Sensors task is not yet running.  Use 'sensors start' to start it\n");
  else
  {
    sensorData_t sensorData;

    if (sensorsCopyData (&sensorData))
    {
      printf ("Sensor count ...... : %d\n", sensorData.sensorCount);
      printf ("ADC changes ....... : %d\n", sensorData.adcChanges);
      printf ("ADC value ......... : %d\n", sensorData.adcValue);
      printf ("ADC value (raw) ... : %d\n", sensorData.adcValueRaw);
      printf ("DAC value ......... : %d\n", sensorData.dacValue);
      printf ("DAC direction ..... : %d\n", sensorData.dacDirection);
    }
    else
      printf ("Unable to get sensor data\n");
  }

  return 0;
}

static int monitorSensorsStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (taskHandles [TASKHANDLE_SENSORS])
    printf ("Sensors task is already running.  Use 'sensors stop' to stop it\n");
  else
    sensorsTaskStart ();

  return 0;
}

static int monitorSensorsStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!sensorsTaskStop ())
    printf ("Sensors task is not yet running.  Use 'sensors start' to start it\n");

  return 0;
}
#endif

#ifdef CFG_SWI
//
//
//
static inline unsigned long monitorSWICommon (unsigned long swi);
static inline unsigned long monitorSWICommon (unsigned long swi)
{
  unsigned long result;

  SWICALL (swi, result);

  return result;
}

static inline unsigned long monitorSWICommon2 (unsigned long swi, unsigned long state);
static inline unsigned long monitorSWICommon2 (unsigned long swi, unsigned long state)
{
  unsigned long result;

  SWICALL1 (swi, state, result);

  return result;
}

static int monitorSWISetAsm (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int state;

  if (!strcmp (argv [0], "on"))
    state = 0;
  else if (!strcmp (argv [0], "off"))
    state = 1;
  else 
  {
    printf ("State must be 'on' or 'off'\n");
    return 0;
  }
  
  printf ("Setting LED2 %s via C, previous state was %s\n", state ? "off" : "on", monitorSWICommon2 (SWICALL_A_LED2SET, state) ? "off" : "on");

  return 0;
}

static int monitorSWIOnAsm (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Setting LED2 on via assembly, previous state was %s\n", monitorSWICommon (SWICALL_A_LED2ON) ? "off" : "on");

  return 0;
}

static int monitorSWIOffAsm (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Setting LED2 off via assembly, previous state was %s\n", monitorSWICommon (SWICALL_A_LED2OFF) ? "off" : "on");

  return 0;
}

static int monitorSWIToggleAsm (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Toggling LED2 via assembly, previous state was %s\n", monitorSWICommon (SWICALL_A_LED2TOGGLE) ? "off" : "on");

  return 0;
}

static int monitorSWISetC (int argc __attribute__ ((unused)), portCHAR **argv)
{
  unsigned int state;

  if (!strcmp (argv [0], "on"))
    state = 0;
  else if (!strcmp (argv [0], "off"))
    state = 1;
  else 
  {
    printf ("State must be 'on' or 'off'\n");
    return 0;
  }
  
  printf ("Setting LED2 %s via C, previous state was %s\n", state ? "off" : "on", monitorSWICommon2 (SWICALL_C_LED2SET, state) ? "off" : "on");

  return 0;
}

static int monitorSWIOnC (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Setting LED2 on via C, previous state was %s\n", monitorSWICommon (SWICALL_C_LED2ON) ? "off" : "on");

  return 0;
}

static int monitorSWIOffC (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Setting LED2 off via C, previous state was %s\n", monitorSWICommon (SWICALL_C_LED2OFF) ? "off" : "on");

  return 0;
}

static int monitorSWIToggleC (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("Toggling LED2 via C, previous state was %s\n", monitorSWICommon (SWICALL_C_LED2TOGGLE) ? "off" : "on");

  return 0;
}
#endif

#ifdef CFG_UIP
//
//
//
static int monitorUIPDHCP (int argc, portCHAR **argv __attribute__ ((unused)))
{
#ifdef CFG_DHCP
  if (!argc)
    printf ("DHCP is %s\n", uipDHCPC  ? "on" : "off");
  else if (!strcmp (argv [0], "on"))
    uipDHCPC = TRUE;
  else if (!strcmp (argv [0], "off"))
    uipDHCPC = FALSE;
  else 
    printf ("State must be 'on' or 'off'\n");
#else
  printf ("DHCP support not enabled\n");
  argc = argc;
#endif

  return 0;
}

#ifdef CFG_TELNETD
static int monitorUIPDrop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  telnetdDisconnect ();

  return 0;
}
#endif

static int monitorUIPGW (int argc, portCHAR **argv __attribute__ ((unused)))
{
  uip_ipaddr_t addr;

  if (!argc)
  {
    uip_getdraddr (&addr);

    printf ("%d.%d.%d.%d\n", uip_ipaddr1 (&addr), uip_ipaddr2 (&addr), uip_ipaddr3 (&addr), uip_ipaddr4 (&addr));
  }
  else
  {
    unsigned int ipaddr;

    if (getIPAddress (argv [0], &ipaddr))
    {
      uip_ipaddr (&addr, (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
      uip_setdraddr (&addr);
    }
  }

  return 0;
}

static int monitorUIPIP (int argc, portCHAR **argv __attribute__ ((unused)))
{
  uip_ipaddr_t addr;

  if (!argc)
  {
    uip_gethostaddr (&addr);

    printf ("%d.%d.%d.%d\n", uip_ipaddr1 (&addr), uip_ipaddr2 (&addr), uip_ipaddr3 (&addr), uip_ipaddr4 (&addr));
  }
  else
  {
    unsigned int ipaddr;

    if (getIPAddress (argv [0], &ipaddr))
    {
      uip_ipaddr (&addr, (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
      uip_sethostaddr (&addr);
    }
  }

  return 0;
}

static int monitorUIPMAC (int argc, portCHAR **argv __attribute__ ((unused)))
{
  if (!argc)
    printf ("%02x:%02x:%02x:%02x:%02x:%02x\n", uip_ethaddr.addr [0], uip_ethaddr.addr [1], uip_ethaddr.addr [2], uip_ethaddr.addr [3], uip_ethaddr.addr [4], uip_ethaddr.addr [5]);
  else if (uipIsRunning ())
    printf ("uIP must be stopped with 'uip stop' before changing MAC address\n");
  else
  {
    int l;

    if ((l = strlen (argv [0]) != 17))
      printf ("MAC address must be formatted as xx:yy:zz:aa:bb:cc\n");
    else
    {
      int i;
      int o;
      struct uip_eth_addr mac;

      for (o = 0, i = 0; i < 6; i++, o += 3)
      {
        unsigned int v;

        argv [0][o + 2] = '\0';

        if (!atoh (&argv [0][o], &v))
        {
          printf ("Badly formatted MAC address: \"%s\"\n", &argv [0][o]);
          return 0;
        }

        mac.addr [i] = v;
      }

      uip_setethaddr (mac);
    }
  }

  return 0;
}

static int monitorUIPNM (int argc, portCHAR **argv __attribute__ ((unused)))
{
  uip_ipaddr_t addr;

  if (!argc)
  {
    uip_getnetmask (&addr);

    printf ("%d.%d.%d.%d\n", uip_ipaddr1 (&addr), uip_ipaddr2 (&addr), uip_ipaddr3 (&addr), uip_ipaddr4 (&addr));
  }
  else
  {
    unsigned int ipaddr;

    if (getIPAddress (argv [0], &ipaddr))
    {
      uip_ipaddr (&addr, (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
      uip_setnetmask (&addr);
    }
  }

  return 0;
}

#ifdef CFG_SNTP
static void monitorUIPSNTPTimeSynced (time_t *epochSeconds)
{
  if (epochSeconds)
  {
    n32_t to;
      
    uip_gettimeoffset (&to);
    *epochSeconds += to;
#ifdef CFG_RTC
    rtcSetEpochSeconds (*epochSeconds);
    printf ("\n");
    monitorRTCGet (0, NULL);
#else
    printf ("\nRTC support not enabled, time is %ld epoch seconds\n", *epochSeconds);
#endif
  }
  else
    printf ("\nCouldn't get time from SNTP server\n");

  printf (PROMPT);
  fflush (stdout);
}

static int monitorUIPSNTP (int argc, portCHAR **argv)
{
  uip_ipaddr_t addr;

  if (!argc)
  {
    uip_getsntpaddr (&addr);

    printf ("%d.%d.%d.%d\n", uip_ipaddr1 (&addr), uip_ipaddr2 (&addr), uip_ipaddr3 (&addr), uip_ipaddr4 (&addr));
  }
  else
  {
    if (!strcmp (argv [0], "set"))
    {
      uip_getsntpaddr (&addr);

      if (uip_iszeroaddr (&addr))
      {
        printf ("NTP server IP address not set\n");
        return 0;
      }
    }
    else
    {
      unsigned int ipaddr;

      if (!getIPAddress (argv [0], &ipaddr))
        return 0;

      uip_ipaddr (&addr, (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
      uip_setsntpaddr (&addr);
    }

    if (!sntpSync (&addr, monitorUIPSNTPTimeSynced))
      printf ("sntpSync() returned 0 (no UDP entries available?)\n");
  }

  return 0;
}
#endif

static int monitorUIPStart (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  switch (uipStart (uipDHCPC))
  {
    case UIPSTATE_ALREADYRUNNING :
      printf ("uIP task is already running.  Use 'uip stop' to stop it\n");
      break;

    case UIPSTATE_TASKDIDNTSTART :
      printf ("uIP task failed to start after 5 seconds (reason not known)\n");
      break;

    case UIPSTATE_ENCJ28C60ERR :
      printf ("ENC28J60 on SPI0 failed to initialize\n");
      break;

    default :
      break;
  }

  return 0;
}

static int monitorUIPStop (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  switch (uipStop ())
  {
    case UIPSTATE_NOTRUNNING :
      printf ("uIP task is not yet running.  Use 'uip start' to start it\n");
      break;

    default : 
      break;
  }

  return 0;
}

static int monitorUIPStats (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  void uip_printstats (void);

  uip_printstats ();

  return 0;
}

#ifdef CFG_SNTP
static int monitorUIPTO (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  if (!argc)
  {
    n32_t to;

    uip_gettimeoffset (&to);
    printf ("Offset is %d seconds from UTC\n", to);
  }
  else
  {
    unsigned int to;

    if (!getNumber (argv [0], &to))
      return 0;

    uip_settimeoffset ((n32_t *) &to);
  }
  return 0;
}
#endif
#endif

#ifdef CFG_WDT
//
//
//
static int monitorWDTTest (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("The watchdog is disabled, by default.  This command enables the watchdog, and if\n"
          "no command is typed for 10 seconds, the watchdog will cause the system to reset.\n"
          "The 10 seconds is based on a 48Mhz PCLK (12Mhz xtal, PLL x 4, VBPDIV = /1).  The\n"
          "'wdt status' command will show the cause of a system reset.  RSIR is reset by\n"
          "this command.\n");

  portDISABLE_INTERRUPTS ();

  SCB_RSIR = SCB_RSIR_MASK;
  WD_MOD = WD_MOD_WDEN | WD_MOD_RESET;
  WD_TC = 120000000;
  WD_FEED = WD_FEED_FEED1;
  WD_FEED = WD_FEED_FEED2;

  portENABLE_INTERRUPTS ();

  return 0;
}

static int monitorWDTStatus (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  int regRSIR = SCB_RSIR;
  int regWDMOD = WD_MOD;
  int regWDTC = WD_TC;
  int regWDTV = WD_TV;

  printf ("Watchdog enabled .................. : %s\n", (regWDMOD & WD_MOD_WDEN) ? "Yes" : "No");
  printf ("Watchdog timeout generates reset .. : %s\n", (regWDMOD & WD_MOD_RESET) ? "Yes" : "No");
  printf ("Watchdog timeout (in PCLK/4's) .... : %d\n", regWDTC);
  printf ("Watchdog timeout counter .......... : %d\n", regWDTV);
  printf ("\n");
  printf ("Reset because of POR .............. : %s\n", (regRSIR & SCB_RSIR_POR) ? "Yes" : "No");
  printf ("Reset because of /RESET ........... : %s\n", (regRSIR & SCB_RSIR_EXTR) ? "Yes" : "No");
  printf ("Reset because of watchdog ......... : %s\n", (regRSIR & SCB_RSIR_WDTR) ? "Yes" : "No");
  printf ("Reset because of BOD .............. : %s\n", (regRSIR & SCB_RSIR_BODR) ? "Yes" : "No");

  return 0;
}

static int monitorWDTClear (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  SCB_RSIR = SCB_RSIR_MASK;
  WD_MOD = 0;

  return 0;
}
#endif

//
//
//
static int monitorVersion (int argc __attribute__ ((unused)), portCHAR **argv __attribute__ ((unused)))
{
  printf ("LPC-P2148 Demo, Version " __VERSION ", " __DATE__ " " __TIME__ "\n");
  printf ("Copyright (c) 2008, J.C. Wren\n");

  return 0;
}

//
//
//
#if defined CFG_CONSOLE_USB || defined CFG_CONSOLE_UART1
static void monitorReassignFD (FILE *fp, int fd);
static void monitorReassignFD (FILE *fp, int fd)
{
  fp->_file = fd;
}
#endif

static portTASK_FUNCTION (vMonitorTask, pvParameters __attribute__ ((unused)))
{
  static U8 buffer [256];
  static portCHAR *argv [34];
  int argc;
  int fd = fileno (stdin);

  fclose (stderr);
  stderr = stdout;

#if defined CFG_CONSOLE_USB
  fd = open ("/dev/usb", O_RDWR);
#elif defined CFG_CONSOLE_UART1
  fd = open ("/dev/uart1", O_RDWR);
#endif

#if defined CFG_CONSOLE_USB || defined CFG_CONSOLE_UART1
  monitorReassignFD (stdin, fd);
  monitorReassignFD (stdout, fd);
#endif

  monitorVersion (0, NULL);

#ifdef CFG_FATFS
  //
  //  If CFG_USB_MSC defined, this gets done by mscblockInit()
  //
  {
    FRESULT f;

    if ((f = diskInitialize (0)) != FR_OK)
      f_printerror (f);
  }
#endif

#ifdef CFG_AUTO_UIP
  uipStart (CFG_AUTO_DHCP_VALUE);
#endif

  for (;;)
  {
    int l;

    if ((l = argsGetLine (fd, buffer, sizeof (buffer), PROMPT)))
    {
#ifdef CFG_RTC
      if ((l == 1) && ((buffer [0] == 0xfe) || (buffer [0] == 0xff)))
      {
        int type = buffer [0];
        time_t now = time (NULL);
        ctime_r (&now, (char *) buffer);
        printf ("%s -- %s", (type == 0xfe) ? "ALARM" : "PERIODIC", buffer);
      }
      else 
#endif
        if (argsParse ((char *) buffer, argv, sizeof (argv), &argc))
        printf ("Too many arguments (max %ld)\n", arrsizeof (argv));
      else if (argv [0])
        argsDispatch (commandList, argc, &argv [0]);
    }

#ifdef CFG_WDT
    portDISABLE_INTERRUPTS ();
    WD_FEED = WD_FEED_FEED1;
    WD_FEED = WD_FEED_FEED2;
    portENABLE_INTERRUPTS ();
#endif
  }
}

//
//
//
signed portBASE_TYPE monitorTaskStart (void)
{
  return xTaskCreate (vMonitorTask,  (const signed portCHAR * const) "Monitor", 1024, NULL, (tskIDLE_PRIORITY + 3), &taskHandles [TASKHANDLE_MONITOR]);
}
