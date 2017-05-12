/****************************************************************************
 
  Header file for ME218C Team LeftShark XBEE UART Module
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef RxSM_H
#define RxSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { WaitFor0x7E, WaitForMSBLen, WaitForLSBLen, 
               ReadDataPacket } RxState_t ;


// Public Function Prototypes

bool InitRxSM ( uint8_t Priority );
bool PostRxSM( ES_Event ThisEvent );
ES_Event RunRxSM( ES_Event ThisEvent );
RxState_t QueryRxSM ( void );


#endif /* RxSM_H */

