//*****************************************************************************
//
// opendrive.c - Example
//
// Base on hello.c, part of revision 6852 of the EK-LM3S6965 Firmware Package.
//
//*****************************************************************************

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "drivers/rit128x96x4.h"
#include "globals.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// A set of flags used to track the state of the application.
//
//*****************************************************************************
unsigned long g_ulFlags;

//*****************************************************************************
//
// The speed of the processor clock, which is therefore the speed of the clock
// that is fed to the peripherals.
//
//*****************************************************************************
unsigned long g_ulSystemClock;

//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     0 - Up
//     1 - Down
//     2 - Left
//     3 - Right
//     4 - Select
//
//*****************************************************************************
unsigned char g_ucSwitches = 0x1f;

enum buttons {
	UP, DOWN, LEFT, RIGHT, SELECT
};
static char* messages[] = { "  Up  ", " Down ", " Left ", "Right ", "Select", "      " };

//*****************************************************************************
//
// The number of clock ticks that have occurred.  This is used as an entropy
// source for the random number generator; the number of clock ticks at the
// time of a button press or release is an entropic event.
//
//*****************************************************************************
static unsigned long g_ulTickCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last screen update
// was requested.  This is used to divide down the system clock tick to the
// desired screen update rate.
//
//*****************************************************************************
static unsigned char g_ucScreenUpdateCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last application
// update was performed.  This is used to divide down the system clock tick to
// the desired application update rate.
//
//*****************************************************************************
static unsigned char g_ucAppUpdateCount = 0;

//*****************************************************************************
//
// The vertical counter used to debounce the push buttons.  The bit positions
// are the same as g_ucSwitches.
//
//*****************************************************************************
static unsigned char g_ucSwitchClockA = 0;
static unsigned char g_ucSwitchClockB = 0;

//*****************************************************************************
//
// Handles the SysTick timeout interrupt.
//
//*****************************************************************************
void SysTickIntHandler(void) {
	unsigned long ulData, ulDelta;

	//
	// Increment the tick count.
	//
	g_ulTickCount++;

	//
	// Indicate that a timer interrupt has occurred.
	//
	HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 1;

    //
    // Increment the screen update count.
    //
    g_ucScreenUpdateCount++;

    //
    // See if 1/2th of a second has passed since the last screen update.
    //
    if(g_ucScreenUpdateCount == (CLOCK_RATE / 2)) {
        //
        // Restart the screen update count.
        //
        g_ucScreenUpdateCount = 0;

        //
        // Invert the GPIO F0 value - toggle LED on/off.
        //
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0,
                     GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) ^ GPIO_PIN_0);
    }

	//
	// Increment the application update count.
	//
	g_ucAppUpdateCount++;

	//
	// See if 1/100th of a second has passed since the last application update.
	//
	if (g_ucAppUpdateCount != (CLOCK_RATE / 100)) {
		//
		// Return without doing any further processing.
		//
		return;
	}

	//
	// Restart the application update count.
	//
	g_ucAppUpdateCount = 0;

	//
	// Read the state of the push buttons.
	//
	ulData = (GPIOPinRead(GPIO_PORTE_BASE,
			(GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)) |
			(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) << 3));

	//
	// Determine the switches that are at a different state than the debounced
	// state.
	//
	ulDelta = ulData ^ g_ucSwitches;

	//
	// Increment the clocks by one.
	//
	g_ucSwitchClockA ^= g_ucSwitchClockB;
	g_ucSwitchClockB = ~g_ucSwitchClockB;

	//
	// Reset the clocks corresponding to switches that have not changed state.
	//
	g_ucSwitchClockA &= ulDelta;
	g_ucSwitchClockB &= ulDelta;

	//
	// Get the new debounced switch state.
	//
	g_ucSwitches &= g_ucSwitchClockA | g_ucSwitchClockB;
	g_ucSwitches |= (~(g_ucSwitchClockA | g_ucSwitchClockB)) & ulData;

	//
	// Determine the switches that just changed debounced state.
	//
	ulDelta ^= (g_ucSwitchClockA | g_ucSwitchClockB);

	//
	// See if the select button was just pressed.
	//
	if ((ulDelta & 0x10) && !(g_ucSwitches & 0x10)) {
		//
		// Set a flag to indicate that the select button was just pressed.
		//
		HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
	}
}

//*****************************************************************************
//
// Handles display output for buttons.
//
//*****************************************************************************
void ButtonPressed(enum buttons key) {
	unsigned char empty = 5;
	//
	// Eye candy ...
	//
	switch (key) {
		case UP:
			RIT128x96x4StringDraw(messages[key], 40, 24, 15);
			RIT128x96x4StringDraw(messages[empty], 60, 32, 15);
			RIT128x96x4StringDraw(messages[empty], 40, 40, 15);
			RIT128x96x4StringDraw(messages[empty], 20, 32, 15);
			break;
		case DOWN:
			RIT128x96x4StringDraw(messages[empty], 40, 24, 15);
			RIT128x96x4StringDraw(messages[empty], 60, 32, 15);
			RIT128x96x4StringDraw(messages[key], 40, 40, 15);
			RIT128x96x4StringDraw(messages[empty], 20, 32, 15);
			break;
		case LEFT:
			RIT128x96x4StringDraw(messages[empty], 40, 24, 15);
			RIT128x96x4StringDraw(messages[empty], 60, 32, 15);
			RIT128x96x4StringDraw(messages[empty], 40, 40, 15);
			RIT128x96x4StringDraw(messages[key], 20, 32, 15);
			break;
		case RIGHT:
			RIT128x96x4StringDraw(messages[empty], 40, 24, 15);
			RIT128x96x4StringDraw(messages[key], 60, 32, 15);
			RIT128x96x4StringDraw(messages[empty], 40, 40, 15);
			RIT128x96x4StringDraw(messages[empty], 20, 32, 15);
			break;
		default:
			break;
	}
}

//*****************************************************************************
//
// Print "Hello from opendrive!" to the OLED on the Stellaris evaluation board.
//
//*****************************************************************************
int main(void) {
	//
	// If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
	// a workaround to allow the PLL to operate reliably.
	//
	if (REVISION_IS_A2) {
		SysCtlLDOSet(SYSCTL_LDO_2_75V);
	}

	//
	// Set the clocking to run at 50MHz from the PLL.
	//
	SysCtlClockSet(
			SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
					| SYSCTL_XTAL_8MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

	//
	// Get the system clock speed.
	//
	g_ulSystemClock = SysCtlClockGet();

	//
	// Enable the peripherals used by the application.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	//
	// Configure the GPIOs used to read the state of the on-board push buttons.
	//
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPadConfigSet(GPIO_PORTE_BASE,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
			GPIO_PIN_TYPE_STD_WPU);

    //
    // Set GPIO F0 as an output. This drives an LED on the board that will
    // toggle.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

	//
	// Configure the first UART for 115,200, 8-N-1 operation.
	//
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTEnable(UART0_BASE);

	//
	// Send a welcome message to the UART.
	//
	UARTCharPut(UART0_BASE, 'W');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, 'l');
	UARTCharPut(UART0_BASE, 'c');
	UARTCharPut(UART0_BASE, 'o');
	UARTCharPut(UART0_BASE, 'm');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, '!');
	UARTCharPut(UART0_BASE, '\r');
	UARTCharPut(UART0_BASE, '\n');

	//
	// Initialize the OLED display.
	//
	RIT128x96x4Init(1000000);

	//
	// Hello!
	//
	RIT128x96x4StringDraw("Hello from opendrive!", 4, 14, 15);

	//
	// Configure SysTick to periodically interrupt.
	//
	SysTickPeriodSet(g_ulSystemClock / CLOCK_RATE);
	SysTickIntEnable();
	SysTickEnable();

    //
    // Throw away any button presses.
    //
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;

	//
	// Loop forever.
	//
	while (1) {
		//
		// See if one of the direction buttons was pressed.
		//
		switch (g_ucSwitches & 0x0f) {
		case 0x0e:
			// 'up' button
			ButtonPressed(UP);
			break;
		case 0x0d:
			// 'down' button
			ButtonPressed(DOWN);
			break;
		case 0x0b:
			// 'left' button
			ButtonPressed(LEFT);
			break;
		case 0x07:
			// 'right' button
			ButtonPressed(RIGHT);
			break;
		default:
			break;
		}

		//
		// See if the select button was pressed.
		//
		if (HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS)) {
			//
			// Clear the button press indicator.
			//
			HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;

			// FIXME - 'select' button action
		}

	}

	return 0;
}
