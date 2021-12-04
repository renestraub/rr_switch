//--- includes ---------------------------------------------------------------

#include <Arduino.h>

#include "simpleos.h"

//--- global functions -------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  SOS_Init(); // Initialize Simple OS, also initializes sub systems
}

void loop()
{
  /* Dispatch all messages */
  SOS_Schedule();

  /* Check whether a timer tick has elapsed */
  if (TMR_HasTickElapsed())
  {
    /* Process timers, fire events if elapsed */
    SOS_TimerTick();
  }
}

//--- eof --------------------------------------------------------------------
