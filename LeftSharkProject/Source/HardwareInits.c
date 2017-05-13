/****************************************************************************
 Module
   HardwareInits.c

 Revision
   1.0.1

 Description
   Contains hardware initalization functions used in DOG/FARMER
	 
 History
 When           Who     What/Why
 -------------- ---     --------
 05/10/2017		ejg		Started coding. Added InitUARTs.

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// Include our own header
#include "HardwareInits.h"

// Common Headers
#include <stdint.h>
#include <stdbool.h>

// Headers for TIVA's GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_uart.h"		// TODO: double check
#include "inc/hw_nvic.h"		// TODO: enable interrupts in the NVIC

// Headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

// Bit definitions
#include "BITDEFS.H"

// Project definitions
#include "DEFINITIONS.h"

/*----------------------------- Module Defines ----------------------------*/




/*---------------------------- Module Functions ---------------------------*/
/* Prototypes for private functions for this service. They should be functions
   relevant to the behavior of this service*/
	 

	/*------------------------------ Module Code ------------------------------*/

	void InitUARTS( void ) {
	
		// Enable UART1 in RCGCUART register (enables clock to UART1)
		HWREG(SYSCTL_RCGCUART) |= SYSCTL_RCGCUART_R1;
		
		// Wait for module to be ready (by polling Bit 1 of PRUART)
		while ((HWREG(SYSCTL_PRUART) & SYSCTL_PRUART_R1) != SYSCTL_PRUART_R1);
		
		// Enable clock to Port B
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
		
		// Wait for GPIO module to be ready (PRGPIO)
		while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
		
		// Configure PB0 (RX) as digital input and PB1 (TX) as digital output
		HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (RX_UARTS | TX_UARTS);
		HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= ~RX_UARTS;
		HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= TX_UARTS;

		// Select Alternate Function (AFSEL) on Port B pins 0 and 1
		HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (RX_UARTS | TX_UARTS);
		
		// Configure PMCn fields in GPIOCTL register to assign UART to PB0 and PB1
		//			This is a mux value of 1 that we want to use for specifying the 
		//			function on pins (bits) 0 & 1
		HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
			(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0xffffff00) + (1<<(0*BitsPerNibble)) +
      (1<<(1*BitsPerNibble));
		
		// Disable UART1 by clearing UARTEN in UARTCTL register
		HWREG(UART1_BASE+UART_O_CTL) &= ~UART_CTL_UARTEN;
		
		// Note: next commands are trickiest. If bugs, check here first (if not, NVIC)
		
		// Write integer portion of Baud Rate Divisor (260) to UARTIBRD
		HWREG(UART1_BASE+UART_O_IBRD) = (HWREG(UART1_BASE+UART_O_IBRD) & ~UART_IBRD_DIVINT_M) + IBRD;
		
		// Write fractional portion of Baud Rate Divisor (27) to UARTFBRD
		HWREG(UART1_BASE+UART_O_FBRD) = (HWREG(UART1_BASE+UART_O_FBRD) & ~UART_FBRD_DIVFRAC_M) + FBRD;
		
		// Write 0x03 to WLEN bits in UARTLCRH to set 8 bit word length
		HWREG(UART1_BASE+UART_O_LCRH) = (HWREG(UART1_BASE+UART_O_LCRH) & ~UART_LCRH_WLEN_M)|UART_LCRH_WLEN_8;  
	  
		// Enable RX, TX, EOT interrupt, and UARTEN in UARTCTL
		HWREG(UART1_BASE+UART_O_CTL) |= (UART_CTL_TXE | UART_CTL_RXE | UART_CTL_EOT | UART_CTL_UARTEN);
		
		// Globally enable interrupts (if not already set) 
		__enable_irq();
	
		// Enable NVIC interrupt for UART1 (Interrupt #6)
		HWREG(NVIC_EN0) |= BIT6HI;
		
		// Just to be safe: Disable UART TXmit interrupt by clearing interrupt mask bit in UART mask reg
		HWREG(UART1_BASE + UART_O_IM) &= ~UART_IM_TXIM;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
