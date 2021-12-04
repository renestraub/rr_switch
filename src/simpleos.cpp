
//--- includes ---------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

// #include "debug.h"          // DBG_ASSERT()
#include "timer.h" // Timer system
#include "io.h"

#include "simpleos.h" // Own header file

extern void MAIN_Process(const SOS_Message *pMsg); // Main process

//--- defines ----------------------------------------------------------------

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
  unsigned int evq_WriteIndex;
  unsigned int evq_ReadIndex;
  unsigned int evq_NumEntries;
  SOS_QueueEntry evq_Messages[EVENT_QUEUE_SIZE];
} SOS_EventQueue;

//--- variables --------------------------------------------------------------

// Message queue, holding all pending events
static SOS_EventQueue sos_Events;

// Timers
static SOS_Timer sos_Timers[Timer_MAX];

static int sos_TickPhase = 0;

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

#ifdef _DEBUG

static const char *sos_GetProcessName(SOS_ProcessId id)
{
  switch (id)
  {
  case Process_Undefined:
    return "<Undefined>";
  case Process_Watchdog:
    return "Watchdog";
  case Process_ExtReset:
    return "ExtReset";
  case Process_Supervisor:
    return "Supervisor";
  case Process_LED:
    return "LED";
  default:
    DBG_ASSERT(False);
    return "<unknown>";
  }
}

#endif

//----------------------------------------------------------------------------

static void sos_TimersInit(void)
{
  SOS_Timer *pTimer = sos_Timers;
  unsigned int i;

  for (i = 0; i < Timer_MAX; i++)
  {
    pTimer->tmr_Active = false;
    pTimer++;
  }
}

//----------------------------------------------------------------------------

static void sos_TimersProcess(void)
{
  SOS_Timer *pTimer = sos_Timers;
  unsigned int i;

  for (i = 0; i < Timer_MAX; i++)
  {
    if (pTimer->tmr_Active)
    {
      if (TMR_HasIntervalElapsed(pTimer->tmr_Time))
      {
        // D(( "Timer %d elapsed at %ld\n", i,
        //   TMR_GetMilliseconds() ));

        SOS_PostEvent(pTimer->tmr_Process, &pTimer->tmr_Msg);

        pTimer->tmr_Active = false;
      }
    }
    pTimer++;
  }
}

//--- global functions ------------------------------------------------------

void SOS_StartTimer(SOS_TimerId timerId, uint32_t milliSecs, SOS_ProcessId processId, const SOS_Message *pMsg)
{
  SOS_Timer *pTimer = &sos_Timers[timerId];

  // DBG_ASSERT( timerId < Timer_MAX );
  // DBG_ASSERT( milliSecs < 1000000UL );
  // DBG_ASSERT( (processId > Process_Undefined) && (processId < Process_MAX) );

  pTimer->tmr_Time = TMR_StartInterval(milliSecs);
  pTimer->tmr_Process = processId;
  pTimer->tmr_Msg = *pMsg;
  pTimer->tmr_Active = true;

  // D(( "Timer %d started at %ld, timeout %ld millisecs\n",
  //   timerId, TMR_GetMilliseconds(), milliSecs ));
}

//----------------------------------------------------------------------------

void SOS_StopTimer(SOS_TimerId timerId)
{
  SOS_Timer *pTimer = &sos_Timers[timerId];

  // DBG_ASSERT( timerId < Timer_MAX );

  pTimer->tmr_Active = false;
  // D(( "Timer %d stopped\n", timerId ));
}

//----------------------------------------------------------------------------

void SOS_PostEvent(SOS_ProcessId processId, const SOS_Message *pMsg)
{
  // DBG_ASSERT( (processId > Process_Undefined) && (processId < Process_MAX) );

  /* Place message in event queue */
  if (sos_Events.evq_NumEntries < EVENT_QUEUE_SIZE)
  {
    sos_Events.evq_Messages[sos_Events.evq_WriteIndex].qe_ProcessId = processId;
    sos_Events.evq_Messages[sos_Events.evq_WriteIndex].qe_Message = *pMsg;

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

void SOS_TimerTick(void)
{
  /* Call processes's Tick function */

  // PWR_Tick();           // Call every 500uS so that each voltage is checked
  //                       // once every 1mS.

  if (sos_TickPhase == 0)
  {
    /* Process OS timers once each millisecond, fire events if elapsed */
    sos_TimersProcess();

    sos_TickPhase = 1;
  }
  else
  {
    /* Handle digital IOs once per millisecond */
    IO_Tick();

    sos_TickPhase = 0;
  }
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
      MAIN_Process(&pEntry->qe_Message);
      break;

    default:
      // D(("Unknown process %d\n", pEntry->qe_ProcessId));
      // DBG_ASSERT(False);
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

  // IO_Init();
}

//--- eof --------------------------------------------------------------------
