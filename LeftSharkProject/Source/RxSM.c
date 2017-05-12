/****************************************************************************
 Module
   RxSM.c

 Revision
   0.0.1

 Description
   This is a ME218C Team LeftShark XBEE UART Module made upon the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/11/17 11:12 afb     Starting Module
 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "RxSM.h"

/*----------------------------- Module Defines ----------------------------*/

#define CHAR_WAIT_PRD   2         //amount of time to wait before signaling a lost connection = between 1 and 2ms)
#define RX_INTERRUPT_M BIT4HI     //RTMIS is bit 4 of UARTMIS
#define RX_DATA_M 0xFF            // to makes first 8 bits of UARTDR  

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

void ClearRxVars (void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static RxState_t CurrentState;
static uint16_t FrameLength = 0;
static uint8_t RxInterruptBit = 0; 
static uint8_t RxData = 0;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitRxSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Drew Bell, 5/11/17, 18:55
****************************************************************************/
bool InitRxSM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
	// call UART Initialization function in another module
	
  // put us into the Initial PseudoState
  CurrentState = WaitFor0x7E;
	
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostRxSM

 Parameters
     ES_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Drew Bell, 05/11/17, 19:25
****************************************************************************/
bool PostRxFSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunRxSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Drew Bell, 05/11/17, 15:23
****************************************************************************/
ES_Event RunRxSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case WaitFor0x7E :       // If current state is initial State
        if ( ThisEvent.EventType == ES_0x7E_RECEIVED )// only respond to ES_Init
        {
            // Change CurrentState to WaitForMSBLen
            CurrentState = WaitForMSBLen;
          
            // Clear receive variables
            ClearRxVars();
          
            // Start a timer to check for lost connection
            ES_Timer_InitTimer(UART_TIMEOUT , CHAR_WAIT_PRD);
          
        }
         break;

    case WaitForMSBLen :       // If current state is WaitForMSBLen
      switch ( ThisEvent.EventType )
      {
        case ES_BYTE_RECEIVED : //If event is a received byte
          
            // Set MSB of Len
            
        
            // Start DoubleCharacter timer
            // Change CurrentState to WaitForLSBLen
        
          break;

        // repeat cases as required for relevant events
        default :
            ; 
      }  // end switch on CurrentEvent
      break;
    // repeat state pattern as required for other states
    default :
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryRxSM

 Parameters
     None

 Returns
     RxState_t The current state of the RxSM state machine

 Description
     returns the current state of the RxSM state machine
 Notes

 Author
     Drew Bell, 05/11/17, 19:21
****************************************************************************/
RxState_t QueryRxSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/


/****************************************************************************
 Function
     ClearRxVars

 Parameters
     None

 Returns
     Nothing

 Description
     clears out receive variables for each new byte
 Notes

 Author
     Drew Bell, 05/11/17, 19:21
****************************************************************************/
void ClearRxVars ( void )
{
  FrameLength = 0;      //clear frame length

}




/***************************************************************************
 Xbee UART Receive Interrupt Service Routine
 ***************************************************************************/

void RxISR (void)
{
  /*since there is only one interrupt vector for each UART Module
    first check if there is a valid receive interrupt */
  
  //poll the Rx interrupt 
  RxInterruptBit = (HWREG(UART1_BASE + UART_O_MIS) & RX_INTERRUPT_M);
  
  //If receive interrupt flag is set in RXRIS in UARTMIS
   if (RxInterruptBit){
      //Clear the source of the interrupt in UARTICR
      HWREG(UART1_BASE + UART_O_MIS) |= RX_INTERRUPT_M;
      //Read the data in UARTDR into NewRxByte
     RxData = (HWREG(UART1_BASE + UART_O_DR) | RX_DATA_M);
      //Read OverRun bit in UARTDR into OverRunFlag
      //Read BreakError bit in UARTDR into BreakErrorFlag
      //Read ParityError bit in UARTDR into ParityErrorFlag
      //Read FramingError bit in UARTDR into FramingErrorFlag


  //If OverRunFlag OR BreakErrorFlag OR ParityErrorFlag OR FramingError is true 
    //Write to UARTECR register to clear error flags (W1C)
    //Post RxConnectionLost event
  //Else (if data is good)
    //If btye is 0x7E
    //Create new event called ThisEvent
    //Set EventType to be NewStartByte
  //Post ThisEvent to RxSM
  //Else if byte is anything else
  //Create new event called ThisEvent
    //Set EventType to be NewDataByte
  //Post ThisEvent to RxSM
  //Endif	
  //Endif
	//Endif 

  
  
  
  
}