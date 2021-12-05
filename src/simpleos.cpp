
//--- includes ---------------------------------------------------------------

#include <stdint.h>

#include "timer.h" // Timer system
#include "io.h"
#include "app.h"

#include "simpleos.h" // Own header file

//--- types ------------------------------------------------------------------

/* Timer structure */
typedef struct _SOS_Timer
{
  bool tmr_Active;           // True if timer is in use
  TMR_Interval tmr_Time;     // Time when timer will elapse
  SOS_ProcessId tmr_Process; // Process to send message to
  SOS_Message tmr_Msg;       // Message to deliver when timer elapses
} SOS_Timer;

/* Event queue entry */
typedef struct _SOS_QueueEntry
{
  SOS_ProcessId qe_ProcessId; // Process that received message
  SOS_Message qe_Message;     // Message
} SOS_QueueEntry;

/* Event queue. Essentially a ring buffer with message entries */
typedef struct _SOS_EventQueue
{
  unsigned evq_WriteIndex;
  unsigned evq_ReadIndex;
  unsigned evq_NumEntries;
  SOS_QueueEntry evq_Messages[EVENT_QUEUE_SIZE];
} SOS_EventQueue;

//--- variables --------------------------------------------------------------

// Message queue, holding all pending events
static SOS_EventQueue sos_Events;

// Timers
static SOS_Timer sos_Timers[Timer_MAX];

//--- local functions --------------------------------------------------------

static SOS_QueueEntry *sos_GetEvent(void)
{
  SOS_QueueEntry *pEntry = 0;

  /* Get pointer to event queue entry (if one is present) */
  if (sos_Events.evq_NumEntries > 0)
  {
    pEntry = &sos_Events.evq_Messages[sos_Events.evq_ReadIndex];

    if (++sos_Events.evq_ReadIndex >= EVENT_QUEUE_SIZE)
    {
      sos_Events.evq_ReadIndex = 0;
    }

    /* Remove entry from queue */
    sos_Events.evq_NumEntries--;
  }
  else
  {
    /* Queue empty */
  }

  return pEntry;
}

//----------------------------------------------------------------------------

static void sos_TimersInit(void)
{
  SOS_Timer *pTimer = sos_Timers;

  for (auto i = 0; i < Timer_MAX; i++)
  {
    pTimer->tmr_Active = false;
    pTimer++;
  }
}

//----------------------------------------------------------------------------

static void sos_TimersProcess(void)
{
  SOS_Timer *pTimer = sos_Timers;

  for (auto i = 0; i < Timer_MAX; i++)
  {
    if (pTimer->tmr_Active)
    {
      if (TMR_HasIntervalElapsed(pTimer->tmr_Time))
      {
        // D(( "Timer %d elapsed at %ld\n", i,
        //   TMR_GetMilliseconds() ));

        SOS_PostEvent(pTimer->tmr_Process, pTimer->tmr_Msg);

        pTimer->tmr_Active = false;
      }
    }
    pTimer++;
  }
}

//--- global functions ------------------------------------------------------

void SOS_StartTimer(SOS_TimerId timerId, uint32_t milliSecs, SOS_ProcessId processId, const SOS_Message &msg)
{
  SOS_Timer *pTimer = &sos_Timers[timerId];

  pTimer->tmr_Time = TMR_StartInterval(milliSecs);
  pTimer->tmr_Process = processId;
  pTimer->tmr_Msg = msg;
  pTimer->tmr_Active = true;

  // D(( "Timer %d started at %ld, timeout %ld millisecs\n",
  //   timerId, TMR_GetMilliseconds(), milliSecs ));
}

//----------------------------------------------------------------------------

void SOS_RestartTimer(SOS_TimerId timerId, uint32_t milliSecs)
{
  SOS_Timer *pTimer = &sos_Timers[timerId];

  pTimer->tmr_Time = TMR_StartInterval(milliSecs);
  pTimer->tmr_Active = true;
}

//----------------------------------------------------------------------------

void SOS_StopTimer(SOS_TimerId timerId)
{
  SOS_Timer *pTimer = &sos_Timers[timerId];

  pTimer->tmr_Active = false;
  // D(( "Timer %d stopped\n", timerId ));
}

//----------------------------------------------------------------------------

void SOS_PostEvent(SOS_ProcessId processId, const SOS_Message &msg)
{
  /* Place message in event queue */
  if (sos_Events.evq_NumEntries < EVENT_QUEUE_SIZE)
  {
    sos_Events.evq_Messages[sos_Events.evq_WriteIndex].qe_ProcessId = processId;
    sos_Events.evq_Messages[sos_Events.evq_WriteIndex].qe_Message = msg;

    if (++sos_Events.evq_WriteIndex >= EVENT_QUEUE_SIZE)
    {
      sos_Events.evq_WriteIndex = 0;
    }

    sos_Events.evq_NumEntries++;
  }
  else
  {
    /* Queue too small !!!!! */
    // D(("SOS_PostEvent() Event queue overflow\n"));
    // DBG_ASSERT(False);
  }
}

//----------------------------------------------------------------------------

void SOS_PostEventArgs(SOS_ProcessId processId, SOS_Event event, uint32_t param)
{
  const SOS_Message msg = {event, param};
  SOS_PostEvent(processId, msg);
}

//----------------------------------------------------------------------------

void SOS_TimerTick(void)
{
  /* Call processes's Tick function */
  IO_Tick();

  /* Process OS timers once each millisecond, fire events if elapsed */
  sos_TimersProcess();
}

//----------------------------------------------------------------------------

void SOS_Schedule(void)
{
  SOS_QueueEntry *pEntry = 0;

  /* Call all run functions unconditionally */

  IO_Run();

  /* Distribute events to processes */

  pEntry = sos_GetEvent();
  while (pEntry != 0)
  {
    // D(("Processing message (Event:%d, Param:%d) for process %s\n",
    //   pEntry->qe_Message.msg_Event,
    //   pEntry->qe_Message.msg_Param1,
    //   sos_GetProcessName(pEntry->qe_ProcessId)));

    switch (pEntry->qe_ProcessId)
    {
    case Process_Main:
      APP_Process(&pEntry->qe_Message);
      break;

    default:
      // D(("Unknown process %d\n", pEntry->qe_ProcessId));
      break;
    }

    pEntry = sos_GetEvent();
  }
}

//----------------------------------------------------------------------------

void SOS_Init(void)
{
  // Initialize OS

  sos_TimersInit();

  // Initialize all known processes

  TMR_Init(); // Start Timer system
  IO_Init();
  APP_Init();
}

//--- eof --------------------------------------------------------------------
