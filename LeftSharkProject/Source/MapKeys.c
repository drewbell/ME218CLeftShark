/****************************************************************************
 Module
   MapKeys.c

 Revision
   1.0.1

 Description
   This service maps keystrokes to events 

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/06/14 14:44 jec      tweaked to be a more generic key-mapper
 02/07/12 00:00 jec      converted to service for use with E&S Gen2
 02/20/07 21:37 jec      converted to use enumerated type for events
 02/21/05 15:38 jec      Began coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include <stdio.h>
#include <ctype.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MapKeys.h"
#include "RxSM.h"


/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMapKeys

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 02/07/12, 00:04
****************************************************************************/
bool InitMapKeys ( uint8_t Priority )
{
  MyPriority = Priority;

  return true;
}

/****************************************************************************
 Function
     PostMapKeys

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMapKeys( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunMapKeys

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   maps keys to Events for HierMuWave Example
 Notes
   
 Author
   J. Edward Carryer, 02/07/12, 00:08
****************************************************************************/
ES_Event RunMapKeys( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
  //static uint8_t key;

    if ( ThisEvent.EventType == ES_NEW_KEY) // there was a key pressed
    {
        
        //print the received character:
        //key = toupper(ThisEvent.EventParam);  
        //printf("\n\rNewKey = %c", key);    
        
        switch ( toupper(ThisEvent.EventParam))
        {
          
            case '1' : ThisEvent.EventType = ES_0x7E_RECEIVED; 
                       break;
            case '2' : ThisEvent.EventType = ES_BYTE_RECEIVED; 
                       ThisEvent.EventParam = 0;    //send MSB length
                       break;
            case '3' : ThisEvent.EventType = ES_BYTE_RECEIVED; 
                       ThisEvent.EventParam = 5;    //sends LSB length
                       break;
            case '4' : ThisEvent.EventType = ES_BYTE_RECEIVED; 
                       ThisEvent.EventParam = 0x22;  //sends random number
                       break;
            case 'E' : ThisEvent.EventType = ES_UART_ERROR_FLAG; 
                       break;		
            
						
        }
        PostRxSM(ThisEvent);
    }
    
  return ReturnEvent;
}
