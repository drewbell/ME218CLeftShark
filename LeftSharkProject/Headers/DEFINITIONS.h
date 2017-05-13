
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/****************************************************************************

Framework, base hardware software, and state machine headers

****************************************************************************/

/* Framework system */
#include "ES_Configure.h"
#include "ES_Framework.h"

/* Base hardware and software */
#include "inc/hw_pwm.h"				// For PWM
#include "inc/hw_memmap.h" 		// For alternate pin functions
#include "inc/hw_timer.h"			// For timers
#include "inc/hw_types.h" 		// For accessing hardware register
#include "inc/hw_gpio.h" 			// For alternate pin functions
#include "inc/hw_sysctl.h" 		// For initializing SSI system
#include "driverlib/gpio.h"  	// For GPIO pin definitions
#include <stdio.h> 						// For debugging via terminal
#include <ctype.h> 						// For standard types (bool, etc)
#include "stdint.h" 					// For using unsigned integer types
#include "BITDEFS.H" 					// For using BIT30HI, etc.
#include "inc/hw_nvic.h" 			// For managing interrupts
#include "termio.h"						// For terminal input/output
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "ES_ShortTimer.h"


/* Service/HSM headers */
//#include "FARMERTXmitSM.h"
//#include "FARMERUARTISR.h"
//#include "DOGTXmitSM.h" do not need for this Tiva
//#include "DOGUARTISR.h" do not need for this Tiva


/****************************************************************************

General hardware registry/timing definitions

****************************************************************************/
#ifndef ALL_BITS
#define ALL_BITS 										(0xff << 2) // For writing to shift register
#endif

#define BitsPerNibble 4

/****************************************************************************

DOGTXmitSM.c

****************************************************************************/
// Parts of a packet
#define START_DELIMITER 0x7E		// always the same
#define LENGTH_MSB 0x00					// always the same
#define API_ID 0x01 						// API identifier for a transmit request
#define FRAME_ID 0x01						// Based on lecture 48 example
#define OPTIONS 0x00						// Prelec 48

//todo: define status bit positions based on protocol (pair bit, brake bit, etc)

// Packet Types
#define REQ_PAIR 0x00 //todo: change later?
#define ENCR_KEY 0x01 //todo: change later?
#define CNTRL 	 0x02 //todo: change later?
#define STATUS   0x03 //todo: change later?
#define RESEND   0x04 //todo: change later?

/* start position of frame data bytes.
because start delimiter is 0, MSB and LSB of length are bytes 1 and 2
*/
#define START_OF_FRAME_DATA 3 

// start position of RF data bytes.
#define START_OF_RF_DATA 8

// overhead for packet size
#define MIN_PACKET_SIZE 9

// overhead for frame data
#define MIN_FRAME_DATA 5

// RF data length
#define REQ_PAIR_SIZE 2 //todo: change later?
#define ENCR_KEY_SIZE 33 //todo: change later?
#define CNTRL_SIZE 5 //todo: change later?
#define STATUS_SIZE 3 //todo: change later?

/****************************************************************************

HardwareInits.c

****************************************************************************/

//------UARTs--------
// Port B pin definitions (UARTs)
#define RX_UARTS GPIO_PIN_0
#define TX_UARTS GPIO_PIN_1

// UARTs definitions
#define IBRD 260			// Integer portion for 9600 baud
#define FBRD 27				// Fractional portion for 9600 baud


#endif /* DEFINITIONS_H */

