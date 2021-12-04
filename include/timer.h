#ifndef _TIMER_H
#define _TIMER_H

/** 
 * @file
 * Timer System based on Arduino millisecond timer.
 */

//--- includes ---------------------------------------------------------------

#include <stdint.h>

//--- types ------------------------------------------------------------------

/**
 * Timer Interval.
 *
 * This structure is returned by a call to TMR_StartInterval() and later used
 * in calls to TMR_HasIntervalElapsed().
 */
typedef uint32_t TMR_Interval;

//--- functions --------------------------------------------------------------

/**
 * Check whether a timer tick period has elapsed since last call.
 *
 * @return True if a timer tick occured since the last call. False
 *         otherwise.
 **/
bool TMR_HasTickElapsed(void);

/**
 * Returns current system time in milliseconds.
 *
 * @return Current time in milliseconds.
 **/
uint32_t TMR_GetMilliseconds(void);

/**
 * Define a timer interval.
 *
 * Creates a new timer interval that will elapse in \em milliSecs
 * from now on.
 * Note: The time is rounded to system ticks. The function ensures that
 *  an interval lasts at least one system tick.
 *
 * @param   milliSecs  IN  Milliseconds when interval shall end.
 * @return  Timer Interval object.
 * @see TMR_HasIntervalElapsed()
 **/
TMR_Interval TMR_StartInterval(uint32_t milliSecs);

/**
 * Checks whether an interval is elapsed.
 *
 * @param  interval  IN  Interval object, which has been created
 *                       by a call to TMR_StartInterval().
 * @return True if interval has elapsed, False otherwise.
 * @see TMR_StartInterval()
 **/
bool TMR_HasIntervalElapsed(TMR_Interval interval);

/**
 * Initializes Timer Module. 
 * Must be called before any other function of this module can be used.
 **/
void TMR_Init();

#endif /* _TIMER_H */

//--- eof --------------------------------------------------------------------
