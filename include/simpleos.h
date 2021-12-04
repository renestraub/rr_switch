#ifndef _SIMPLE_OS_H
#define _SIMPLE_OS_H

/**
 * @file
 * State/Event based Simple Operating System
 */

//--- includes ---------------------------------------------------------------

#include <stdint.h>

#include "timer.h"

//--- defines ----------------------------------------------------------------

/* Queue upto 16 messages */
#define EVENT_QUEUE_SIZE (16)

//--- types ------------------------------------------------------------------

/* List of all processes */
enum SOS_ProcessId
{
  Process_Undefined = 0,
  Process_Main,
  Process_MAX
};

/* List of available timers */
enum SOS_TimerId
{
  Timer_Undefined = 0,
  Timer_Main50,
  Timer_Main250,
  Timer_MAX
};

/* SOS Events */
enum SOS_Event
{
  Evt_None,
  Evt_KeyMode,
  Evt_KeyLeft,
  Evt_KeyRight,
  Evt_TickerServo,
  Evt_TickerLED,
  Evt_MAX
};

struct SOS_Message
{
  SOS_Event msg_Event; // Event code
  uint32_t msg_Param1; // Optional message parameter
};

//--- functions --------------------------------------------------------------

/**
 * Starts a timer.
 *
 * Use this function to starts a timer event. The timer is not reoccurring,
 * it only triggers once.
 *
 * @param  timerId    IN  Id of timer.
 * @param  milliSecs  IN  Number of milliseconds for timer to run.
 * @param  processId  IN  Id of process to receive message.
 * @param  pMsg       IN  Pointer to message consisting of event and
 *                        message paramters.
 * @see SOS_StopTimer
 */
void SOS_StartTimer(SOS_TimerId timerId, uint32_t milliSecs, SOS_ProcessId processId, const SOS_Message *pMsg);

/**
 * Stops a timer.
 *
 * Use this function to stop a timer started with SOS_StartTimer().
 *
 * @param  timerId   IN  Id of timer.
 * @note   If the timer just fired and placed a message in the OS's message
 *  queue, that message is not destroyed. This means that even after the
 *  timer has been stopped a message can arrive.
 * @see SOS_StartTimer
 */
void SOS_StopTimer(SOS_TimerId timerId);

/**
 * Posts an event to a process.
 *
 * This function posts a message to a process. The message is stored
 * in the OS's message queue. It is delivered after this function returns,
 * when the scheduler runs the next time.
 *
 * @param  processId  IN  Id of process to receive message.
 * @param  pMsg       IN  Pointer to message consisting of event and
 *                        message paramters. The message is copied in this
 *                        call and can thus be released when the function
 *                        returns.
 **/
void SOS_PostEvent(SOS_ProcessId processId, const SOS_Message *pMsg);

/**
 * OS Timer Tick.
 * Call this function on each timer tick. The OS processes its timers
 * in this function and calls the tick() function of all processes.
 **/
void SOS_TimerTick(void);

/**
 * OS Schedule Function.
 * Call this function to process messages and call the run() function
 * of all processes.
 **/
void SOS_Schedule(void);

/**
 * Initializes Simple OS.
 **/
void SOS_Init(void);

#endif /* _SIMPLE_OS_H */

//--- eof --------------------------------------------------------------------
