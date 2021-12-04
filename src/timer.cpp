
//--- includes ---------------------------------------------------------------

#include <Arduino.h> // millis()

#include "timer.h" // Own header file

//--- variables --------------------------------------------------------------

static uint32_t time_ms = 0;

//--- global functions ------------------------------------------------------

bool TMR_HasTickElapsed(void)
{
  uint32_t now = millis();

  if (now > time_ms)
  {
    time_ms = now;
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------

uint32_t TMR_GetMilliseconds(void)
{
  return millis();
}

//----------------------------------------------------------------------------

TMR_Interval TMR_StartInterval(uint32_t milliSecs)
{
  TMR_Interval iv;

  uint32_t now = TMR_GetMilliseconds();
  iv = now + milliSecs + 1; // +1 to ensure minimal delay of 1 tick

  return iv;
}

//----------------------------------------------------------------------------

bool TMR_HasIntervalElapsed(TMR_Interval interval)
{
  uint32_t now = TMR_GetMilliseconds();
  uint32_t delta = now - interval;

  return (delta < 0x80000000UL);
}

//----------------------------------------------------------------------------

void TMR_Init()
{
}

//--- eof --------------------------------------------------------------------
