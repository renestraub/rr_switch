
//--- includes ---------------------------------------------------------------

#include <Arduino.h>        // io access

#include "simpleos.h"       // SOS_PostEvent()

#include "io.h"             // Own header file



//--- defines ----------------------------------------------------------------

/* Defines bits in io_OutputState variable */
#define PIN_KEY_LEFT_N      2
#define PIN_KEY_RIGHT_N     3
#define PIN_KEY_MODE_N      4


/* Possible values for digital input */
typedef enum _IO_DigInValue
{
  IO_IN_LOW = 0,            // Pin is low (0)
  IO_IN_HIGH                // Pin is high (1)
}
IO_DigInValue;

/* State variables for digital inputs */
typedef struct _IO_DigInState
{
  uint8_t         di_ShiftReg;
  bool            di_ChangeFlag;
  IO_DigInValue   di_CurrValue;
  unsigned int    di_TimeActive;
}
IO_DigInState;

/* Digital inputs mapping table - Routes inputs messages to processes */
typedef struct _IO_EventMap
{
  SOS_ProcessId   em_ProcessId;
  SOS_Event       em_Event;
  unsigned int    em_AutoRepeatInMs;
  unsigned int    em_ParamLow;
  unsigned int    em_ParamHigh;
}
IO_EventMap;


//--- variables --------------------------------------------------------------

/* Shadow register for digital outputs */
// static UInt16         io_OutputState    = 0;      

/* Digital input states */
static IO_DigInState  io_InputState[IO_Inputs_Max];

/* Message map for digital inputs (do not change order) */
static const IO_EventMap io_InputMsgMap[IO_Inputs_Max] =
{
  // Process to notify  Event            AutoRep      Change to '0'     Change to '1'
  { Process_Main,   Evt_KeyLeft,         50,          IO_INPUT_INACTIVE,  IO_INPUT_ACTIVE }, 
  { Process_Main,   Evt_KeyRight,        50,          IO_INPUT_INACTIVE,  IO_INPUT_ACTIVE }, 
  { Process_Main,   Evt_KeyMode,         0,           IO_INPUT_INACTIVE,  IO_INPUT_ACTIVE }, 
};

//--- local functions --------------------------------------------------------

inline static void scanInputPin( unsigned int index, bool currState )
{
  IO_DigInState*      pState  = &io_InputState[index];
  const IO_EventMap*  pEvent  = &io_InputMsgMap[index];

  /* Scan input */
  pState->di_ShiftReg <<= 1;
  if (currState) {
    pState->di_ShiftReg |= 1;
  }

  /* Check for a stable state */
  switch ( pState->di_ShiftReg & 0x07 )
  {
  case 0:   // Remains low
    break;

  case 7:   // Remains high
    if (pEvent->em_AutoRepeatInMs != 0) {
      pState->di_TimeActive += 1;
      if (pState->di_TimeActive == pEvent->em_AutoRepeatInMs) {
        pState->di_TimeActive = 0;
        // pState->di_CurrValue  = IO_IN_HIGH;
        pState->di_ChangeFlag = true;
      }
    }      
    break;

  case 3:   // Changes to high
    if ( pState->di_CurrValue != IO_IN_HIGH )
    {
      // D(("DigInput %s changed to High\n", inputToString(index)));
      pState->di_CurrValue  = IO_IN_HIGH;
      pState->di_ChangeFlag = true;
      pState->di_TimeActive = 0;
    }
    break;

  case 4:   // Changes to low
    if ( pState->di_CurrValue != IO_IN_LOW )
    {
      // D(("DigInput %s changed to Low\n", inputToString(index)));
      pState->di_CurrValue  = IO_IN_LOW;
      pState->di_ChangeFlag = true;
    }
    break;
  
  default:  // Bouncing
//    D(("DigInput %d bounces\n", index));
    break;
  }
}

//----------------------------------------------------------------------------

inline static void scanAllInputs( void )
{
  scanInputPin(IO_KEY_LEFT, digitalRead(PIN_KEY_LEFT_N) == 0);
  scanInputPin(IO_KEY_RIGHT_N, digitalRead(PIN_KEY_RIGHT_N) == 0);
  scanInputPin(IO_KEY_MODE_N, digitalRead(PIN_KEY_MODE_N) == 0);
}

//----------------------------------------------------------------------------

static void reportChanges( void )
{
  const IO_EventMap*    pEvent  = io_InputMsgMap;
  IO_DigInState*        pState  = io_InputState;
  unsigned int          i;

  /* For each input that changed state send message to listener process */
  for ( i=0; i<IO_Inputs_Max; i++ )
  {
    if (    (pState->di_ChangeFlag) 
         && (pEvent->em_ProcessId != Process_Undefined)
       )
    {
      SOS_Message msg;

      msg.msg_Event   = pEvent->em_Event;
      switch (pState->di_CurrValue)
      {
        case IO_IN_LOW:     msg.msg_Param1  = pEvent->em_ParamLow;  break;
        case IO_IN_HIGH:    msg.msg_Param1  = pEvent->em_ParamHigh; break;
        default:            break;
      }

      Serial.println("state changed");
      // D(("Input state change: %s = %d\n",
      //   inputToString(i), pState->di_CurrValue));
      SOS_PostEvent( pEvent->em_ProcessId, &msg );

      pState->di_ChangeFlag = false;
    }

    pEvent++;
    pState++;
  }
}

//----------------------------------------------------------------------------

static void updateOutputs( void )
{
//   UInt16 portB;

//   portB = PORTB;
//   portB &= ~( (1<<6) | (1<<7) | (1<<12) | (1<<13) | (1<<14) | (1<<15) );

//   if ( (io_OutputState & PIN_ENIO_N) == 0 ) {
//     portB |= 1<<13;
//   }
//   if ( (io_OutputState & PIN_ENLPIO_N) == 0 ) {
//     portB |= 1<<15;
//   }
//   if ( (io_OutputState & PIN_ENLPCORE_N) == 0 ) {
//     portB |= 1<<14;
//   }
//   if ( (io_OutputState & PIN_PWRRESET_N) == 0 ) {
//     portB |= 1<<7;
//   }
//   if ( (io_OutputState & PIN_PMWPWROK) == PIN_PMWPWROK ) {
//     portB |= 1<<12;
//   }
//   if ( (io_OutputState & PIN_DEBUG_LED_N) == 0 ) {
//     portB |= 1<<6;
//   }
//   PORTB = portB;
}

//----------------------------------------------------------------------------

// inline static void modifyState( UInt16 bitMask, Bool state )
// {
//   if (state) {
//     io_OutputState |=  bitMask;
//   }
//   else {
//     io_OutputState &= ~bitMask;
//   }
//   updateOutputs();
// }

//----------------------------------------------------------------------------

static void configureOutputs( void )
{
  // /*
  //  * Set default values. Note that these are the logical signal states.
  //  * Inversion for active low outputs takes place in updateOutputs()
  //  *  - All supplies are initially off
  //  *  - Power Reset is activated
  //  *  - PMC Power Ok indication is off
  //  *  - LED is off
  //  */
  // io_OutputState    = PIN_PWRRESET_N;
  // updateOutputs();
}

//----------------------------------------------------------------------------

static void configureInputs( void )
{
  pinMode(PIN_KEY_LEFT_N, INPUT_PULLUP);
  pinMode(PIN_KEY_RIGHT_N, INPUT_PULLUP);
  pinMode(PIN_KEY_MODE_N, INPUT_PULLUP);
}

//--- global functions ------------------------------------------------------

// void IO_SetEnIO( Bool state )
// {
//   modifyState( PIN_ENIO_N, state );
// }

// //----------------------------------------------------------------------------

// void IO_SetEnLpIo( Bool state )
// {
//   modifyState( PIN_ENLPIO_N, state );
// }

// //----------------------------------------------------------------------------

// void IO_SetEnLpCore( Bool state )
// {
//   modifyState( PIN_ENLPCORE_N, state );
// }

// //----------------------------------------------------------------------------

// void IO_SetPwrReset( Bool state )
// {
//   modifyState( PIN_PWRRESET_N, state );
// }

// //----------------------------------------------------------------------------

// void IO_SetPmcPwrOk( Bool state )
// {
//   modifyState( PIN_PMWPWROK, state );
// }
// //----------------------------------------------------------------------------

// void IO_SetDebugLED( Bool state )
// {
//   modifyState( PIN_DEBUG_LED_N, state );
// }


//----------------------------------------------------------------------------

void IO_ForceStateUpdate( IO_Inputs input )
{
  // DBG_ASSERT( input < IO_Inputs_Max );

  // D(("Requesting status report of input %s\n",
  //   inputToString( input ) ));
  io_InputState[input].di_ChangeFlag = true;

  reportChanges();
}


//--- OS interface -----------------------------------------------------------

void IO_Tick( void )
{
  /* This function is called once per system tick */

  /* Check digital inputs (debounce, report state change) */

  scanAllInputs();
  reportChanges();
}

//----------------------------------------------------------------------------

void IO_Run( void )
{
  /* Called on every process loop */

  updateOutputs();
}

//----------------------------------------------------------------------------

void IO_Init( void )
{
  IO_DigInState*  pState = io_InputState;
  unsigned int    i;

  /* Initialize PIC hardware */
  configureInputs();
  configureOutputs();

  /* Initialize Input state registers (assume input is low) */
  for ( i=0; i<IO_Inputs_Max; i++ )
  {
    pState->di_ShiftReg     = 0x00;
    pState->di_ChangeFlag   = false;
    pState->di_CurrValue    = IO_IN_LOW;
    pState->di_TimeActive   = 0;

    pState++;
  }

  /* Load current pin values to get initial state */
  for ( i=0; i<3; i++ )
  {
    scanAllInputs();
  }

  /* Clear the changed flag to suppress initial state change reports */
  pState = io_InputState;
  for ( i=0; i<IO_Inputs_Max; i++ )
  {
    pState->di_ChangeFlag   = false;
    pState++;
  }
}

//--- eof --------------------------------------------------------------------
