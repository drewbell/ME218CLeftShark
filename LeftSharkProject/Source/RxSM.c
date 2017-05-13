/****************************************************************************
 Module
   RxSM.c

 Revision
   0.0.1

 Description
   This is a ME218C Team LeftShark XBEE UART Module made upon the
   Gen2 Events and Services Framework.

  Notes: 
  
  5/12 - need to establish proper length of connection loss timer, UART_TIMEOUT

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
#include "HardwareInits.h"

/*----------------------------- Module Defines ----------------------------*/

#define CONNECTION_TIMEOUT_PRD   1000            // amount of time to wait before signaling a lost connection = 1 second (1000ms)
#define RX_DATA_M   0xFF                // to makes first 8 bits of UARTDR 
#define CLR_UART_ERR_FLAGS    0xFF
#define XBEE_START_DELIMITER  0x7E   
#define ALL_BITS_HI     0xFF
#define LONGEST_PACKET_LENGTH   0x96    // placeholder for longest packet length
#define NUM_OVERHEAD_BYTES  4           // counts start delimiter, MSB length, LSB length, ChkSum

//ifdef defines
//#define RxTestPrints
#define PrintRecdPacket


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

void ClearRxVars (void);
void PrintUARTErrors (void);
void ClearRxDataPacket ( void );

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match that of enum in header file
static RxState_t CurrentState;
static uint8_t FrameLengthMSB = 0;
static uint8_t FrameLengthLSB = 0;
static uint8_t PacketLength = 0;
static uint16_t BytesLeft = 0;
static uint16_t RxArrayIndex = 0;      //which byte we are working with in the RxDataPacket array
static uint8_t RxInterruptBit = 0; 
static uint8_t OverRunBit = 0;
static uint8_t BreakErrorBit = 0;
static uint8_t ParityErrorBit = 0;
static uint8_t FramingErrorBit = 0;
static uint8_t ChkSum = 0;
static uint8_t XbeeChkSum = 0;
static uint8_t RxDataByte = 0;

//The RxDataPacket is an array of 8-bit bytes that includes the Xbee start delimiter, two length bits, frame data,
// and checksum.  
static uint8_t RxDataPacket[LONGEST_PACKET_LENGTH]; 


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
    InitUARTS();
    
    //enable interrupts via the UART interrupt mask register
	HWREG(UART1_BASE + UART_O_IM) |= UART_IM_RXIM;
    
    //clear the variables to start
    ClearRxVars();
	
    // put us into the Initial PseudoState
    CurrentState = WaitFor0x7E;
    
    #ifdef RxTestPrints
            printf("\n\rInit to WaitFor0x7E State");
    #endif
	
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
bool PostRxSM( ES_Event ThisEvent )
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
        if ( ThisEvent.EventType == ES_0x7E_RECEIVED ) {// only respond to ES_Init
            // Change CurrentState to WaitForMSBLen
            CurrentState = WaitForMSBLen;
            
            #ifdef RxTestPrints
            printf("\n\rGood Start Delimiter:   WaitFor0x7E --> WaitForMSBLen State");
            #endif
          
            // Clear receive variables
            ClearRxVars();
            
            //place RxDataByte into RxDataPacket and increment RxArrayIndex
            RxDataPacket[RxArrayIndex] = RxDataByte;
            RxArrayIndex++;
          
            // Start Connection Timeout timer
            //ES_Timer_InitTimer(UART_TIMEOUT , CONNECTION_TIMEOUT_PRD);
        }
        else if ( ThisEvent.EventType == ES_UART_ERROR_FLAG)
          {
            //call UART print error function and stay in WaitFor0x7E state
            PrintUARTErrors();
          }
         break;

    case WaitForMSBLen :       // If current state is WaitForMSBLen
        switch ( ThisEvent.EventType ) {

            case ES_BYTE_RECEIVED : //If event is a received byte
                // Set MSB of Length to the value event parameter sent from the ISR
                FrameLengthMSB = ThisEvent.EventParam;
                //place RxDataByte into RxDataPacket and increment RxArrayIndex
                RxDataPacket[RxArrayIndex] = RxDataByte;
                RxArrayIndex++;
            
                // Start Connection Timeout timer
                //ES_Timer_InitTimer(UART_TIMEOUT , CONNECTION_TIMEOUT_PRD);
                // Change CurrentState to WaitForLSBLen
                CurrentState = WaitForLSBLen;
            
                #ifdef RxTestPrints
                printf("\n\rGood MSB:   WaitForMSBLen --> WaitForLSBLen State");
                #endif
            break;
          
            case ES_TIMEOUT : //If EventType of ThisEvent is timeout
                //Change CurrentState to WaitFor0x7E
                CurrentState = WaitFor0x7E;
                #ifdef RxTestPrints
                printf("\n\rTimeout:    WaitForMSBLen --> WaitFor0x7E State");
                #endif  
            break;
                  
            case ES_UART_ERROR_FLAG : //If EventType of ThisEvent is ES_UART_ERROR_FLAG
                //Change to WaitFor0x7E state
                CurrentState = WaitFor0x7E;
                //Print error messages based on error type
                PrintUARTErrors();
                
                #ifdef RxTestPrints
                printf("\n\rUART Error:  WaitForMSB --> WaitFor0x7E State");
                #endif
            break;  //break for EventType switch
        }   //end switch on CurrentEvent
      break;    //break for WaitForMSBLen


    case WaitForLSBLen :       // If current state is WaitForLSBLen
        switch ( ThisEvent.EventType ) {
        
            case ES_BYTE_RECEIVED : //If event is a received byte
                // Set LSB of Length to the value event parameter sent from the ISR
                FrameLengthLSB = ThisEvent.EventParam;
                //place RxDataByte into RxDataPacket and increment RxArrayIndex
                RxDataPacket[RxArrayIndex] = RxDataByte;
                RxArrayIndex++;
                //Combine MSB and LSB into BytesLeft, then calculate a message length variable
                BytesLeft = 0; 
                BytesLeft = ( (FrameLengthMSB<<8) | FrameLengthLSB );
                PacketLength = BytesLeft + NUM_OVERHEAD_BYTES;
                
                // Start Connection Timeout timer
                //ES_Timer_InitTimer(UART_TIMEOUT , CONNECTION_TIMEOUT_PRD);
            
                // Change CurrentState to ReadDataPacket
                CurrentState = ReadDataPacket;
                #ifdef RxTestPrints
                printf("\n\rGood LSB:   WaitForLSBLen --> ReadDataPacket State");
                #endif
            break;
          
            case ES_TIMEOUT : //If EventType of ThisEvent is timeout
                //Change CurrentState to WaitFor0x7E
                CurrentState = WaitFor0x7E;
                #ifdef RxTestPrints
                printf("\n\rTimeout:  WaitForLSB --> WaitFor0x7E State");
                #endif
            break;
                  
            case ES_UART_ERROR_FLAG : //If EventType of ThisEvent is ES_UART_ERROR_FLAG
                //Change to WaitFor0x7E state
                CurrentState = WaitFor0x7E;
                //Print error messages based on error type
                PrintUARTErrors();
                #ifdef RxTestPrints
                printf("\n\rUART Error:  WaitForMLSB --> WaitFor0x7E State");
                #endif
                break;
        }   //end switch on CurrentEvent
        break;  //break for WaitForLSBLen
        
    case ReadDataPacket : //CurrentState is ReadDataPacket
        #ifdef RxTestPrints
        printf("\n\rEntered ReadDataPacket");
        #endif
        //If EventType of ThisEvent is Byte Received AND BytesLeft NOT EQUAL to zero
        if( (ThisEvent.EventType == ES_BYTE_RECEIVED) && (BytesLeft > 0) ){       
            //place RxDataByte into RxDataPacket
            RxDataPacket[RxArrayIndex] = RxDataByte;
            
            #ifdef RxTestPrints
            printf("    DataByte Read = %i", RxDataPacket[RxArrayIndex]);
            #endif
            
            //Increment RxArray for next position and decrement BytesLeft to get ready for next loop
            RxArrayIndex++;
            BytesLeft--;

            #ifdef RxTestPrints
            printf("    BytesLeft = %i",BytesLeft);
            #endif            
            
            // Add DataByte to ChkSum
            ChkSum = ChkSum + RxDataByte;
            
            // Start Connection Timeout timer
            //ES_Timer_InitTimer(UART_TIMEOUT , CONNECTION_TIMEOUT_PRD);
        } 
        
        //If EventType of ThisEvent is Byte Received AND BytesLeft EQUAL to zero
        else if( (ThisEvent.EventType == ES_BYTE_RECEIVED) && (BytesLeft == 0) ) {
            //place RxDataByte into RxDataPacket
            RxDataPacket[RxArrayIndex] = RxDataByte;
            
            #ifdef RxTestPrints
            printf("\n\rRead CheckSum = %i", RxDataPacket[RxArrayIndex]);
            #endif
            
            // Pull XbeeChkSum out of the last index of RxDataPacket
            XbeeChkSum = RxDataPacket[RxArrayIndex];
            // Add DataByte to ChkSum
            ChkSum = ChkSum + RxDataByte;
            // Subract running checksum from 0xFF to get the final checksum
            ChkSum = ALL_BITS_HI - ChkSum;
            
            //TEST ONLY
            XbeeChkSum = ChkSum;
            
            //If Chksum is bad
            if ( XbeeChkSum != ChkSum ){
                //Change states to WaitFor0x7E
                CurrentState = WaitFor0x7E;
                #ifdef RxTestPrints
                printf("\n\rChkSum Mismatch:  ReadDataPacket --> WaitFor0x7E State");
                #endif
            }
            //Else if Chksum is good
            else if ( XbeeChkSum == ChkSum ) {
                //Post PacketReceived event
                /***** add post function ****/
                       
                //change to WaitFor0x7E to wait for next packet
                CurrentState = WaitFor0x7E;
                
                #ifdef RxTestPrints
                printf("\n\rPacket Received");
                #endif
                
                #ifdef PrintRecdPacket
                for (uint8_t i = 0 ; i < PacketLength ; i++)
                    printf("\n\r%x", RxDataPacket[i]);     

                printf("\n\rEOT*****************\n\n\r");
                #endif
                
                
                
                #ifdef RxTestPrints
                printf("\n\rPacket Complete: Head to WaitFor0x7E State");
                #endif
            }
        }

        //If EventType of ThisEvent is Timeout
        else if( ThisEvent.EventType == ES_TIMEOUT){
            //Change states to WaitFor0x7E
            CurrentState = WaitFor0x7E;
            #ifdef RxTestPrints
            printf("\n\rTimeout:  ReadDataPacket --> WaitFor0x7E State");
            #endif
        }

        //If EventType of ThisEvent is ES_UART_ERROR_FLAG
        else if( ThisEvent.EventType == ES_UART_ERROR_FLAG ){
            //Change to WaitFor0x7E state
            CurrentState = WaitFor0x7E;
            //Print error messages based on error type
            PrintUARTErrors();
            
            #ifdef RxTestPrints
            printf("\n\rUART Error: ReadDataPacket --> WaitFor0x7E State");
            #endif
        }
        
        break;      
    }      // end switch on Current State
  
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
  FrameLengthMSB = 0;   //clear frame length variables
  FrameLengthLSB = 0;
  PacketLength = 0;
  ChkSum = 0;           //clear checksums
  XbeeChkSum = 0;
  ClearRxDataPacket();
  RxArrayIndex = 0;     //clear count of which byte we are workign with in the RxDataPacket array
    
}

/****************************************************************************
 Function
     ClearRxDataPacket

 Parameters
     None

 Returns
     Nothing

 Description
     clears out receive data packet for each new packet
 Notes

 Author
     Drew Bell, 05/12/17, 19:21
****************************************************************************/
void ClearRxDataPacket ( void )
{
  for(uint8_t i = 0 ; i < LONGEST_PACKET_LENGTH ; i++){
      RxDataPacket[i] = 0;
  }
 
}


/****************************************************************************
 Function
     PrintUARTErrors

 Parameters
     None

 Returns
     Nothing

 Description
     prints error messages based on which UART errors occur
 Notes

 Author
     Drew Bell, 05/11/17, 19:21
****************************************************************************/
void PrintUARTErrors ( void )
{
 
  //If overRun error bit is set, print overrun error msg
  if (OverRunBit) 
  {
      printf("\n\rOverRun Error in UART Rx : Connection Lost");
  }
  
  // if break error bit is set, print break error msg
  if (BreakErrorBit) 
  {
      printf("\n\rBreak Error in UART Rx : Connection Lost");
	}
  
  // if parity error bit is set, print parity error msg
  if (ParityErrorBit)  
  {
		  printf("\n\rParity Error in UART Rx : Connection Lost");
  }
  
  // if framing error bit is set, print framing error msg
  if (FramingErrorBit)	
  {
      printf("\n\rFraming Error in UART Rx : Connection Lost");
  }
  
  //clear error bits
    OverRunBit = 0;
    BreakErrorBit = 0; 
    ParityErrorBit = 0; 
    FramingErrorBit = 0; 

return;

}


/***************************************************************************
 Xbee UART Receive Interrupt Service Routine
 ***************************************************************************/

void RxISR (void)
{
  /*since there is only one interrupt vector for each UART Module
    first check if there is a valid receive interrupt */
    
  //poll the Rx interrupt 
  RxInterruptBit = (HWREG(UART1_BASE + UART_O_MIS) & UART_MIS_RXMIS);
  
  //If receive interrupt flag is set in RXMIS in UARTMIS
   if (RxInterruptBit){
       
       //Clear the source of the interrupt in UARTICR
       HWREG(UART1_BASE + UART_O_ICR) |= UART_ICR_RXIC;
       //Clear the RxInterruptBit 
       RxInterruptBit = 0; 
       //Read the data in UARTDR into NewRxByte
       RxDataByte = ( HWREG(UART1_BASE + UART_O_DR) );       
       //Read OverRun bit in UARTDR into OverRunBit
       OverRunBit = ( HWREG(UART1_BASE + UART_O_RSR) & UART_RSR_OE );
       //Read BreakError bit in UARTDR into BreakErrorBit
       BreakErrorBit = ( HWREG(UART1_BASE + UART_O_RSR) & UART_RSR_BE );
       //Read ParityError bit in UARTDR into ParityErrorBit
       ParityErrorBit = ( HWREG(UART1_BASE + UART_O_RSR) & UART_RSR_PE );
       //Read FramingError bit in UARTDR into FramingErrorBit
       FramingErrorBit = ( HWREG(UART1_BASE + UART_O_RSR) & UART_RSR_FE );

       //If OverRunFlag OR BreakErrorFlag OR ParityErrorFlag OR FramingError is true 
       if ( OverRunBit | BreakErrorBit | ParityErrorBit | FramingErrorBit) {
           //Write to UARTECR register to clear error flags 
           HWREG(UART1_BASE + UART_O_ECR) |= UART_ECR_DATA_M; 
           //Post ES_UART_ERROR_FLAG event to RxSM
           ES_Event ThisEvent;
           ThisEvent.EventType = ES_UART_ERROR_FLAG; 
           PostRxSM(ThisEvent); 
       }
       //Else (if data is good)
       else {
            //If btye is 0x7E
            if ( RxDataByte == XBEE_START_DELIMITER){
                //Create new event called ThisEvent
                ES_Event ThisEvent;
                //Set EventType to be NewStartByte 
                ThisEvent.EventType = ES_0x7E_RECEIVED;
                //Post ThisEvent to RxSM
                PostRxSM(ThisEvent);
            }
            //Else the next byte is a data byte
            else {
                //Create new event called ThisEvent
                ES_Event ThisEvent;
                //Set EventType to be NewDataByte and EventParam to be the new byte received
                ThisEvent.EventType = ES_BYTE_RECEIVED;
                ThisEvent.EventParam = RxDataByte;
                //Post ThisEvent to RxSM
                PostRxSM(ThisEvent);
            }
        }
    }
}

