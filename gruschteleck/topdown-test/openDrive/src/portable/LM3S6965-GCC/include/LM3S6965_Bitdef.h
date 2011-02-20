#ifndef __LM3S6965_Bitdef_H 
#define __LM3S6965_Bitdef_H



// Register 4: Raw Interrupt Status (RIS), offset 0x050
#define SYSCTL_RIS_BORRIS 1					// Brown-Out Reset Raw Interrupt Status (default: 0)
#define SYSCTL_RIS_PLLLRIS 6				// PLL Lock Raw Interrupt Status (default: 0)

// Register 8: Run-Mode Clock Configuration (RCC), offset 0x060
#define SYSCTL_RCC_MOSCDIS 0				// Main Oscillator Disable (default: 1)
#define SYSCTL_RCC_IOSCDIS 1				// Internal Oscillator Disable (default: 0)
#define SYSCTL_RCC_OSCSRC 4					// Oscillator Source (default: 0x1)
#define SYSCTL_RCC_XTAL 6					// Crystal Value (default: 0xB)
#define SYSCTL_RCC_BYPASS 11				// PLL Bypass (default: 1)
#define SYSCTL_RCC_PWRDN 13					// PLL Power Down (default: 1)
#define SYSCTL_RCC_PWMDIV 17				// PWM Unit Clock Divisor (default: 0x7)
#define SYSCTL_RCC_USEPWMDIV 20				// Enable PWM Clock Divisor (default: 0)
#define SYSCTL_RCC_USESYSDIV 22				// Enable System Clock Divider (default: 0)
#define SYSCTL_RCC_SYSDIV 23				// System Clock Divisor (default: 0xF)
#define SYSCTL_RCC_ACG 27					// Auto Clock Gating (default: 0)


// Register 9: XTAL to PLL Translation (PLLCFG), offset 0x064
#define SYSCTL_PLLCFG_R 0					// PLL R Value
#define SYSCTL_PLLCFG_F 5					// PLL F Value


// Register 10: Run-Mode Clock Configuration 2 (RCC2), offset 0x070
#define SYSCTL_RCC2_OSCSRC2 4				// Oscillator Source (default: 0x1)
#define SYSCTL_RCC2_BYPASS2 11				// Bypass PLL (default: 1)
#define SYSCTL_RCC2_PWRDN2 13				// Power-Down PLL (default: 1)
#define SYSCTL_RCC2_SYSDIV2 23				// System Clock Divisor (default: 0x0F)
#define SYSCTL_RCC2_USERCC2 31				// Use RCC2 (default: 0)





#endif /* end __LM3S6965_Bitdef_H */