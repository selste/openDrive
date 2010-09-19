//
//  $Id: kbd.c 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/kbd/kbd.c $
//

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../main.h"
#include "kbd.h"

//
//  Define this if using the Pactec keyboard.  The connector will plug straight
//  in, but requires a function to untangle the bits.  If undef'ed and the
//  attached keyboard has nicely ordered pins, we can just read all the pins at
//  once.
//
#define KBD_PACTEC

//
//
//
#ifdef KBD_PACTEC
#define KBD_ROW1        GPIO_IO_P16 // Row 1 (input)
#define KBD_ROW2        GPIO_IO_P17 // Row 2 (input)
#define KBD_COL2        GPIO_IO_P18 // Column 2 (output)
#define KBD_COL1        GPIO_IO_P19 // Column 1 (output)
#define KBD_COL4        GPIO_IO_P20 // Column 4 (output)
#define KBD_COL3        GPIO_IO_P21 // Column 3 (output)
#define KBD_ROW4        GPIO_IO_P22 // Row 4 (input)
#define KBD_ROW3        GPIO_IO_P23 // Row 3 (input)
#else
#define KBD_ROW1        GPIO_IO_P16 // Row 1 (input)
#define KBD_ROW2        GPIO_IO_P17 // Row 2 (input)
#define KBD_ROW3        GPIO_IO_P18 // Row 3 (input)
#define KBD_ROW4        GPIO_IO_P19 // Row 4 (input)
#define KBD_COL1        GPIO_IO_P20 // Column 1 (output)
#define KBD_COL2        GPIO_IO_P21 // Column 2 (output)
#define KBD_COL3        GPIO_IO_P22 // Column 3 (output)
#define KBD_COL4        GPIO_IO_P23 // Column 4 (output)
#define KBD_ROW_SHIFT   16
#endif

#define KBD_ROW_MASK  (KBD_ROW1 | KBD_ROW2 | KBD_ROW3 | KBD_ROW4)

//
//  If CFG_KBD is not defined, we still compile the code, it's just not included.
//  Define TASKHANDLE_KBD so we compile.  Normally this is defined in main.h,
//  but it's #ifndef'ed out so that we don't allocate a task handle slot for it
//  when keyboard support isn't included.
//
#ifndef CFG_KBD
#define TASKHANDLE_KBD 0
#endif

//
//  Structure for passing messages back from the keyboard
//
typedef struct
{
  char key;
} 
xKeyMessage_t;

//
//  The queue for keys
//
static xQueueHandle xKeyQueue;

//
//
//
#define delay166ns() do { asm volatile ("push {r0}\r\npop {r0}\r\n"); } while (0)

//
//
//
static inline void kbdDelay1us (void)
{
  delay166ns ();
  delay166ns ();
  delay166ns ();
  delay166ns ();
  delay166ns ();
  delay166ns ();
}

static void kbdDelay10us (void)
{
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
  kbdDelay1us ();
}

static void kbdDelay100us (void)
{
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
  kbdDelay10us ();
}

static void kbdDelay (void)
{
  kbdDelay100us ();
}

#ifdef KBD_PACTEC
static inline int kbdReadRow (void)
{
  int i = 0;

  if (GPIO1_FIOPIN & KBD_ROW1)
    i |= 1;
  if (GPIO1_FIOPIN & KBD_ROW2)
    i |= 2;
  if (GPIO1_FIOPIN & KBD_ROW3)
    i |= 4;
  if (GPIO1_FIOPIN & KBD_ROW4)
    i |= 8;

  return i;
}
#else
#define kbdReadRow() ((GPIO1_FIOPIN & KBD_ROW_MASK) >> KBD_ROW_SHIFT)
#endif

//
//  Contains short-circuit logic so if no key is pressed (99.9999% of the time)
//  then we return immediately.  Makes the (rash) assumption that there was at
//  least a 100us delay since the previous call, to allow the lines to settle.
//
//  The column drivers are normally low, so if any key is pressed, we'll detect
//  it immediately.  If this is the case, then we bring the column drivers
//  high, and start walking a 0 across them, checking each column for a press.
//  We then return a 16 bit map that has a 1 for each key that is pressed.
//
static int kbdGetKeys (void)
{
  unsigned int keys;

  if ((GPIO1_FIOPIN & KBD_ROW_MASK) == KBD_ROW_MASK)
    return 0;

  GPIO1_FIOSET = (KBD_COL3 | KBD_COL2 | KBD_COL1);
  kbdDelay ();
  keys = kbdReadRow () << 12;
  GPIO1_FIOSET = KBD_COL4;

  GPIO1_FIOCLR = KBD_COL3;
  kbdDelay ();
  keys |= (kbdReadRow () << 8);
  GPIO1_FIOSET = KBD_COL3;

  GPIO1_FIOCLR = KBD_COL2;
  kbdDelay ();
  keys |= (kbdReadRow () << 4);
  GPIO1_FIOSET = KBD_COL2;

  GPIO1_FIOCLR = KBD_COL1;
  kbdDelay ();
  keys |= (kbdReadRow () << 0);

  GPIO1_FIOCLR = (KBD_COL4 | KBD_COL3 | KBD_COL2 | KBD_COL1);

  return keys ^ 0xffff;
}

//
//
//
int kbdGetKey (void)
{
  xKeyMessage_t xKeyMessage;

  if (!uxQueueMessagesWaiting (xKeyQueue))
    return 0;

  xQueueReceive (xKeyQueue, &xKeyMessage, portMAX_DELAY);

  return xKeyMessage.key;
}

int kbdGetKeyBlock (void)
{
  xKeyMessage_t xKeyMessage;

  xQueueReceive (xKeyQueue, &xKeyMessage, portMAX_DELAY);

  return xKeyMessage.key;
}

int kbdGetKeyTimed (int ms)
{
  xKeyMessage_t xKeyMessage;

  if (xQueueReceive (xKeyQueue, &xKeyMessage, ms / portTICK_RATE_MS) == pdTRUE)
    return xKeyMessage.key;

  return 0;
}

//
//
//
void kbdInit (void)
{
  GPIO1_FIODIR &= ~(KBD_ROW4 | KBD_ROW3 | KBD_ROW2 | KBD_ROW1); // Inputs
  GPIO1_FIODIR |=  (KBD_COL4 | KBD_COL3 | KBD_COL2 | KBD_COL1); // Outputs
  GPIO1_FIOCLR  =  (KBD_COL4 | KBD_COL3 | KBD_COL1 | KBD_COL1); // Set outputs low

  if (!xKeyQueue)
    xKeyQueue = xQueueCreate (16, sizeof (xKeyMessage_t));
}

//
//  Scan keypad every 10ms
//
static portTASK_FUNCTION (vKbdTask, pvParameters __attribute__ ((unused)))
{
  portTickType xLastWakeTime;
  unsigned char switchStateIndex = 0;
  unsigned int switchState [10];
  unsigned int lastState = 0;
  const unsigned char keyConvert [] = "DCBA*9630852#741";

  kbdInit ();

  xLastWakeTime = xTaskGetTickCount ();

  for (;;)
  {
    unsigned int i;
    unsigned int j;

    switchState [switchStateIndex++] = kbdGetKeys ();

    if (switchStateIndex >= arrsizeof (switchState))
      switchStateIndex = 0;

    for (j = -1, i = 0; i < arrsizeof (switchState); i++)
      j &= switchState [i];

    if ((lastState != j) && (lastState = j))
    {
      if (!(lastState & (lastState - 1)))
      {
        for (i = 0; i < 16; i++)
        {
          if (lastState & (1 << i))
          {
            xKeyMessage_t xKeyMessage;
            xKeyMessage.key = keyConvert [i];
            xQueueSend (xKeyQueue, &xKeyMessage, 0);
          }
        }
      }
    }
  
    vTaskDelayUntil (&xLastWakeTime, 20 / portTICK_RATE_MS);
  }
}

signed portBASE_TYPE kbdTaskStart (void)
{
  return xTaskCreate (vKbdTask, (const signed portCHAR * const) "KBD", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1), &taskHandles [TASKHANDLE_KBD]);
}

signed portBASE_TYPE kbdTaskStop (void)
{
  if (!taskHandles [TASKHANDLE_KBD])
    return 0;

  vTaskDelete (taskHandles [TASKHANDLE_KBD]);
  taskHandles [TASKHANDLE_KBD] = NULL;
  return 1;
}
