
//--- includes ---------------------------------------------------------------

#include <Arduino.h> // io access

#include "simpleos.h" // SOS_PostEvent()

#include "terminal.h" // Own header file

//--- defines ----------------------------------------------------------------


//--- types ------------------------------------------------------------------


//--- variables --------------------------------------------------------------


//--- local functions --------------------------------------------------------


//--- OS interface -----------------------------------------------------------

void TERM_Run(void)
{
  /* Called on every process loop */

  /* See if we received a character on the USB terminal */
  if (Serial.available() > 0) {
    uint8_t incomingByte = Serial.read();

    SOS_Message msg;
    msg.msg_Event = Evt_Char;
    msg.msg_Param1 = incomingByte;
    SOS_PostEvent(Process_Main, msg);
  }
}

//----------------------------------------------------------------------------

void TERM_Init(void)
{
  /* Nothing to do here */
}

//--- eof --------------------------------------------------------------------
