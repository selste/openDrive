//
//  $Id: lcd_8bit.c 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/lcd/lcd_8bit.c $
//

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../main.h"
#include "lcd.h"

//
//  This is where the various LCD control pins are assigned.  Currently they're
//  all assigned on port 1.  If you have to break them up, you'll need to edit
//  the functions them.  It's very convienent to have the D7..D0 lines grouped
//  together.  Otherwise you'll have to bit test each bit and assign the pin.
//  Much slower.
//
#define LCD_D0        GPIO_IO_P16
#define LCD_D1        GPIO_IO_P17
#define LCD_D2        GPIO_IO_P18
#define LCD_D3        GPIO_IO_P19
#define LCD_D4        GPIO_IO_P20
#define LCD_D5        GPIO_IO_P21
#define LCD_D6        GPIO_IO_P22
#define LCD_D7        GPIO_IO_P23
#define LCD_DATA      (LCD_D7 | LCD_D6 | LCD_D5 | LCD_D4 | LCD_D3 | LCD_D2 | LCD_D1 | LCD_D0)
#define LCD_Dx_SHIFT  ((unsigned int) 16)
#define LCD_RS        GPIO_IO_P24
#define LCD_RW        GPIO_IO_P25
#define LCD_E         GPIO_IO_P26

//
//
//
#define LCD_COMMAND_CLEAR       0x01
#define LCD_COMMAND_HOME        0x02
#define LCD_COMMAND_MODE        0x04
#define LCD_COMMAND_DISPLAY     0x08
#define LCD_COMMAND_SHIFT       0x10
#define LCD_COMMAND_FUNCTION    0x20
#define LCD_COMMAND_CGRAM       0x40
#define LCD_COMMAND_DDRAM       0x80

#define LCD_MODE_SHIFT_NO       0x00
#define LCD_MODE_SHIFT_YES      0x01
#define LCD_MODE_DECREMENT      0x00
#define LCD_MODE_INCREMENT      0x02
#define LCD_MODE_MASK           0x03

#define LCD_SHIFT_CURSOR        0x00
#define LCD_SHIFT_DISPLAY       0x04
#define LCD_SHIFT_LEFT          0x00
#define LCD_SHIFT_RIGHT         0x08
#define LCD_SHIFT_MASK          0x0c

#define LCD_FUNCTION_FONT_5X8   0x00
#define LCD_FUNCTION_FONT_5X11  0x04
#define LCD_FUNCTION_LINES_1    0x00
#define LCD_FUNCTION_LINES_2    0x08
#define LCD_FUNCTION_BUS_4      0x00
#define LCD_FUNCTION_BUS_8      0x10
#define LCD_FUNCTION_MASK       0x1c

#define LCD_DISPLAY_BLINK_OFF   0x00
#define LCD_DISPLAY_BLINK_ON    0x01
#define LCD_DISPLAY_CURSOR_OFF  0x00
#define LCD_DISPLAY_CURSOR_ON   0x02
#define LCD_DISPLAY_DISPLAY_OFF 0x00
#define LCD_DISPLAY_DISPLAY_ON  0x04
#define LCD_DISPLAY_MASK        0x07

#define LCD_PWM_FREQ 20000

//
//  If CFG_LCD is not defined, we still compile the code, it's just not included.
//  Define TASKHANDLE_LCD so we compile.  Normally this is defined in main.h,
//  but it's #ifndef'ed out so that we don't allocate a task handle slot for it
//  when LCD support isn't included.
//
#ifndef CFG_LCD
#define TASKHANDLE_LCD 0
#endif

//
//  Structure for passing messages to the LCD
//
typedef struct
{
  const char *pcMessage;
} 
xLCDMessage_t;

//
//
//
static xQueueHandle xLCDQueue;

//
//  Each operation is 83.33ns.  Cumlative delays make be longer, but not shorter.
//  This delay assumes a 48Mhz clock and MAM=partial
//
#define delay166ns() do { asm volatile (" push {r0}" : /* no output */ : /* no inputs */ ); \
                          asm volatile (" pop  {r0}" : /* no output */ : /* no inputs */ ); \
                        } while (0)

//
//  166ns measured
//
static inline void lcdDelay140ns (void)
{
  delay166ns ();
}

//
//  333ns measured
//
static inline void lcdDelay320ns (void)
{
  delay166ns ();
  delay166ns ();
}

//
//  500ns measured
//
static inline void lcdDelay450ns (void)
{
  delay166ns ();
  delay166ns ();
  delay166ns ();
}

//
//  666ns measured
//
static inline void lcdDelay550ns (void)
{
  delay166ns ();
  delay166ns ();
  delay166ns ();
  delay166ns ();
}

static inline void lcdDataToPort (unsigned char c)
{
  GPIO1_FIOCLR = (~c & 0xff) << LCD_Dx_SHIFT;
  GPIO1_FIOSET = ( c & 0xff) << LCD_Dx_SHIFT;
}

static inline unsigned int lcdDataFromPort (void)
{
  return (GPIO1_FIOPIN & LCD_DATA) >> LCD_Dx_SHIFT;
}

static inline void lcdSelectCommandRegister (void)
{
  GPIO1_FIOCLR = LCD_RS;
}

static inline void lcdSelectDataRegister (void)
{
  GPIO1_FIOSET = LCD_RS;
}

static inline void lcdSelectWriteMode (void)
{
  GPIO1_FIOCLR = LCD_RW;
}

static inline void lcdSelectReadMode (void)
{
  GPIO1_FIOSET = LCD_RW;
}

static inline void lcdEnableHigh (void)
{
  GPIO1_FIOSET = LCD_E;
}

static inline void lcdEnableLow (void)
{
  GPIO1_FIOCLR = LCD_E;
}

static inline void lcdEnableStrobe (void)
{
  lcdEnableHigh ();
  lcdDelay450ns ();
  lcdEnableLow ();
  lcdDelay550ns ();
}

static inline void lcdDataAsInputs (void)
{
  GPIO1_FIODIR &= ~LCD_DATA;
}

static inline void lcdDataAsOutputs (void)
{
  GPIO1_FIODIR |= LCD_DATA;
}

//
//
//
static void lcdWriteData (unsigned char c)
{
  lcdSelectDataRegister ();
  lcdSelectWriteMode ();

  lcdEnableHigh ();
  lcdDataToPort (c);
  lcdDelay140ns ();
  lcdEnableLow ();
  lcdDelay550ns ();
  lcdDelay550ns ();
}

static void lcdWriteCommand (unsigned char c)
{
  lcdSelectCommandRegister ();
  lcdSelectWriteMode ();

  lcdEnableHigh ();
  lcdDataToPort (c);
  lcdDelay140ns ();
  lcdEnableLow ();
  lcdDelay550ns ();
  lcdDelay550ns ();
}

static int lcdReadStatus (void)
{
  unsigned char c;

  lcdSelectCommandRegister ();
  lcdSelectReadMode ();

  lcdEnableHigh ();
  lcdDataAsInputs ();
  lcdDelay140ns ();
  c = lcdDataFromPort ();
  lcdEnableLow ();
  lcdSelectWriteMode ();
  lcdDataAsOutputs ();
  lcdDelay550ns ();
  lcdDelay550ns ();
  
  return c;
}

static void lcdWaitWhileBusy (void)
{
  while (lcdReadStatus () & 0x80)
    ;
}

//
//
//
static void lcdDisplay (unsigned char display)
{
  lcdWaitWhileBusy ();
  lcdWriteCommand (LCD_COMMAND_DISPLAY | (display & LCD_DISPLAY_MASK));
}

static void lcdMode (unsigned char mode)
{
  lcdWaitWhileBusy ();
  lcdWriteCommand (LCD_COMMAND_MODE | (mode & LCD_MODE_MASK));
}

static void lcdClear (void)
{
  lcdWaitWhileBusy ();
  lcdWriteCommand (LCD_COMMAND_CLEAR);
}

static void lcdHome (void)
{
  lcdWaitWhileBusy ();
  lcdWriteCommand (LCD_COMMAND_HOME);
}

static void lcdGotoXY (unsigned int x, unsigned int y)
{
  lcdWaitWhileBusy ();
  lcdWriteCommand (LCD_COMMAND_DDRAM | (y ? 0x40 : 0x00) | (x & 0x0f));
  lcdWaitWhileBusy ();
}

static void lcdPutChar (char c)
{
  lcdWaitWhileBusy ();
  lcdWriteData (c);
}

static void lcdWriteStringCtl (const char *s)
{
  while (*s)
  {
    char c = *s++;

    switch (c)
    {
      case '\f' :
        lcdClear ();
        break;

      case 0x1e :
        lcdHome ();
        break;

      case 0x1b :
        {
          switch (*s++)
          {
            case '=' :
              {
                unsigned char x = *s++;
                unsigned char y = *s++;

                lcdGotoXY (x, y);
              }
              break;

            case '*' :
              lcdClear ();
              break;

            case '.' :
              {
                switch (*s++)
                {
                  case '0' : 
                    lcdDisplay (LCD_DISPLAY_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
                    break;

                  case '1' : 
                    lcdDisplay (LCD_DISPLAY_DISPLAY_ON | LCD_DISPLAY_CURSOR_ON | LCD_DISPLAY_BLINK_ON);
                    break;

                  case '2' : 
                    lcdDisplay (LCD_DISPLAY_DISPLAY_ON | LCD_DISPLAY_CURSOR_ON | LCD_DISPLAY_BLINK_OFF);
                    break;
                }
              }
          }
        }
        break;

      default : 
        lcdPutChar (c);
        break;
    }
  }
}

//
//
//
static void lcdDelayMs (unsigned int timeInMs)
{
  vTaskDelay (timeInMs / portTICK_RATE_MS);
}

//
//
//
static void lcdControllerInit (void)
{
  lcdWriteCommand (LCD_COMMAND_FUNCTION | LCD_FUNCTION_BUS_8 | LCD_FUNCTION_LINES_2 | LCD_FUNCTION_FONT_5X8);  lcdDelayMs (30);
  lcdWriteCommand (LCD_COMMAND_FUNCTION | LCD_FUNCTION_BUS_8 | LCD_FUNCTION_LINES_2 | LCD_FUNCTION_FONT_5X8);  lcdDelayMs (30);
  lcdWriteCommand (LCD_COMMAND_FUNCTION | LCD_FUNCTION_BUS_8 | LCD_FUNCTION_LINES_2 | LCD_FUNCTION_FONT_5X8);  lcdDelayMs (30);
  lcdWriteCommand (LCD_COMMAND_FUNCTION | LCD_FUNCTION_BUS_8 | LCD_FUNCTION_LINES_2 | LCD_FUNCTION_FONT_5X8);  lcdDelayMs (30);

  lcdDisplay (LCD_DISPLAY_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF);
  lcdClear ();
  lcdMode (LCD_MODE_INCREMENT | LCD_MODE_SHIFT_NO);
  lcdGotoXY (0, 0);
}

void lcdWriteChar (char c)
{
  static char buffer [2];

  buffer [0] = c;
  buffer [1] = '\0';

  lcdWriteText (buffer);
}

void lcdWriteText (const char *s)
{
  xLCDMessage_t xLCDMessage;
  
  xLCDMessage.pcMessage = s;

  xQueueSend (xLCDQueue, &xLCDMessage, portMAX_DELAY);
}

void lcdCursorXY (int x, int y)
{
  static char buffer [5];

  buffer [0] = '\033';
  buffer [1] = '=';
  buffer [2] = x;
  buffer [3] = y;
  buffer [4] = '\0';

  lcdWriteText (buffer);
}

//
//
//
void lcdInit (void)
{
  GPIO1_FIODIR |= (LCD_E | LCD_RS | LCD_RW);

  lcdEnableLow ();
  lcdSelectWriteMode ();
  lcdSelectCommandRegister ();
  lcdDataAsOutputs ();
  lcdDataToPort (0xff);

  //
  //  Queue used to send LCD task messages, only 1 outstanding message
  //  (so static buffers, such as those used in lcdCursorAt() don't
  //  get lost)
  //
  if (!xLCDQueue)
    xLCDQueue = xQueueCreate (1, sizeof (xLCDMessage_t));
}

//
//
//
static portTASK_FUNCTION (vLCDTask, pvParameters __attribute__ ((unused)))
{
  lcdInit ();
  lcdControllerInit ();

  for (;;)
  {
    xLCDMessage_t xMessage;
      
    xQueueReceive (xLCDQueue, &xMessage, portMAX_DELAY);

    lcdWriteStringCtl (xMessage.pcMessage);
  }
}

//
//
//
signed portBASE_TYPE lcdTaskStart (void)
{
  return xTaskCreate (vLCDTask, (const signed portCHAR * const) "LCD", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &taskHandles [TASKHANDLE_LCD]);
}

signed portBASE_TYPE lcdTaskStop (void)
{
  if (!taskHandles [TASKHANDLE_LCD])
    return 0;

  vTaskDelete (taskHandles [TASKHANDLE_LCD]);
  taskHandles [TASKHANDLE_LCD] = NULL;
  return 1;
}
