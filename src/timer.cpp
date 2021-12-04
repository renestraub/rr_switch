
//--- includes ---------------------------------------------------------------

#include <Arduino.h>        // millis()

// #include "debug.h"          // DBG_ASSERT()
#include "timer.h"          // Own header file



//--- defines ----------------------------------------------------------------



//--- variables --------------------------------------------------------------

// static bool             tmr_Initialized     = false;    // True if timer system is ready

// static uint32_t           tmr_TickRate        = 0;        // Desired timer tick (i.e 100Hz)

// static uint32_t           tmr_Frequency       = 0;        // Frequency of timer unit
// static uint32_t           tmr_FrequencyBy1000 = 0;        // Frequency of timer unit divided by 1000
// static uint32_t           tmr_Period          = 0;        // Period value for timer to
//                                                         // reach desired tick rate

// static uint32_t           tmr_TickMultiplier  = 0;        // Tick/Millisecond conversion
// static uint32_t           tmr_TickDivider     = 0;

// static volatile uint32_t  tmr_TickCnt         = 0;        // Ticker counter incremented by ISR
// static volatile bool    tmr_IrqScheduled    = false;    // Semaphore set by ISR to indicate
//                                                         // it just executed.
// static volatile bool    tmr_TickElapsed     = false;

static uint32_t time_ms = 0;



//--- local functions --------------------------------------------------------



//--- global functions ------------------------------------------------------

bool TMR_HasTickElapsed( void )
{
  uint32_t now = millis();

  if ( now > time_ms )
  {
    time_ms = now;
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------

// inline uint32_t TMR_GetTicks( void )
// {
//   uint32_t now = millis();
//   return now;
// }

//----------------------------------------------------------------------------

uint32_t TMR_GetMilliseconds( void )
{
  uint32_t now = millis();
  return now;
}

//----------------------------------------------------------------------------

TMR_Interval TMR_StartInterval( uint32_t milliSecs )
{
  TMR_Interval  iv;
  // uint32_t        ticks;
  uint32_t        now;

  // DBG_ASSERT( tmr_Initialized );
  // DBG_ASSERT( milliSecs < (1000UL*3600UL) );

  now     = TMR_GetMilliseconds();
  iv      = now + milliSecs + 1;    // +1 to ensure minimal delay of 1 tick

  return iv;
}

//----------------------------------------------------------------------------

bool TMR_HasIntervalElapsed( TMR_Interval interval )
{
  uint32_t  delta;
  uint32_t  now;

  // DBG_ASSERT( tmr_Initialized );

  now     = TMR_GetMilliseconds();
  delta   = now - interval;

  return ( delta < 0x80000000UL );
}

//----------------------------------------------------------------------------

// void TMR_DelayMs( uint32_t milliSecs )
// {
//   TMR_Interval iv;

//   // DBG_ASSERT( tmr_Initialized );
//   // DBG_ASSERT( milliSecs < (1000UL*3600UL) );

//   iv = TMR_StartInterval( milliSecs );

//   while ( ! TMR_HasIntervalElapsed( iv ) ) {};
// }

//----------------------------------------------------------------------------

void TMR_Init()
{
}

//--- eof --------------------------------------------------------------------
