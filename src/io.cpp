
//--- includes ---------------------------------------------------------------

#include <Arduino.h> // io access

#include "simpleos.h" // SOS_PostEvent()

#include "io.h" // Own header file

//--- defines ----------------------------------------------------------------

/* Defines bits in io_OutputState variable */
#define PIN_DEBUG_LED (1 << 0)

/* Input Pins */
#define PIN_KEY_LEFT_N (2)
#define PIN_KEY_RIGHT_N (3)
#define PIN_KEY_MODE_N (4)

//--- types ------------------------------------------------------------------

/* Possible values for digital input */
typedef enum _IO_DigInValue
{
  IO_IN_LOW = 0, // Pin is low (0)
  IO_IN_HIGH     // Pin is high (1)
} IO_DigInValue;

/* State variables for digital inputs */
typedef struct _IO_DigInState
{
  uint8_t di_ShiftReg;
  bool di_ChangeFlag;
  IO_DigInValue di_CurrValue;
  unsigned di_TimeActive;
} IO_DigInState;

/* Digital inputs mapping table - Routes inputs messages to processes */
typedef struct _IO_EventMap
{
  SOS_ProcessId em_ProcessId;
  SOS_Event em_Event;
  unsigned em_AutoRepeatInMs;
  unsigned em_ParamLow;
  unsigned em_ParamHigh;
} IO_EventMap;

//--- variables --------------------------------------------------------------

/* Shadow register for digital outputs */
static uint16_t io_OutputState = 0;

/* Digital input states */
static IO_DigInState io_InputState[IO_Inputs_Max];

/* Message map for digital inputs (do not change order) */
static const IO_EventMap io_InputMsgMap[IO_Inputs_Max] =
    {
        // Process to notify, Event, AutoRepeat, Change to '0', Change to '1'
        {Process_Main, Evt_KeyLeft, 50, IO_INPUT_INACTIVE, IO_INPUT_ACTIVE},
        {Process_Main, Evt_KeyRight, 50, IO_INPUT_INACTIVE, IO_INPUT_ACTIVE},
        {Process_Main, Evt_KeyMode, 0, IO_INPUT_INACTIVE, IO_INPUT_ACTIVE},
};

//--- local functions --------------------------------------------------------

inline static void scanInputPin(unsigned index, bool currState)
{
  IO_DigInState *pState = &io_InputState[index];
  const IO_EventMap *pEvent = &io_InputMsgMap[index];

  /* Scan input */
  pState->di_ShiftReg <<= 1;
  if (currState)
  {
    pState->di_ShiftReg |= 1;
  }

  /* Check for a stable state */
  switch (pState->di_ShiftReg & 0x07)
  {
  case 0: // Remains low
    break;

  case 7: // Remains high
    if (pEvent->em_AutoRepeatInMs != 0)
    {
      pState->di_TimeActive += 1;
      if (pState->di_TimeActive == pEvent->em_AutoRepeatInMs)
      {
        pState->di_TimeActive = 0;
        // pState->di_CurrValue  = IO_IN_HIGH;
        pState->di_ChangeFlag = true;
      }
    }
    break;

  case 3: // Changes to high
    if (pState->di_CurrValue != IO_IN_HIGH)
    {
      // D(("DigInput %s changed to High\n", inputToString(index)));
      pState->di_CurrValue = IO_IN_HIGH;
      pState->di_ChangeFlag = true;
      pState->di_TimeActive = 0;
    }
    break;

  case 4: // Changes to low
    if (pState->di_CurrValue != IO_IN_LOW)
    {
      // D(("DigInput %s changed to Low\n", inputToString(index)));
      pState->di_CurrValue = IO_IN_LOW;
      pState->di_ChangeFlag = true;
    }
    break;

  default: // Bouncing
    break;
  }
}

//----------------------------------------------------------------------------

inline static void scanAllInputs(void)
{
  scanInputPin(IO_KEY_LEFT, digitalRead(PIN_KEY_LEFT_N) == 0);
  scanInputPin(IO_KEY_RIGHT_N, digitalRead(PIN_KEY_RIGHT_N) == 0);
  scanInputPin(IO_KEY_MODE_N, digitalRead(PIN_KEY_MODE_N) == 0);
}

//----------------------------------------------------------------------------

static void reportChanges(void)
{
  const IO_EventMap *pEvent = io_InputMsgMap;
  IO_DigInState *pState = io_InputState;

  /* For each input that changed state send message to listener process */
  for (auto i = 0; i < IO_Inputs_Max; i++)
  {
    if ((pState->di_ChangeFlag) && (pEvent->em_ProcessId != Process_Undefined))
    {
      SOS_Message msg;

      msg.msg_Event = pEvent->em_Event;
      switch (pState->di_CurrValue)
      {
      case IO_IN_LOW:
        msg.msg_Param1 = pEvent->em_ParamLow;
        break;

      case IO_IN_HIGH:
        msg.msg_Param1 = pEvent->em_ParamHigh;
        break;

      default:
        break;
      }

      // D(("Input state change: %s = %d\n",
      //   inputToString(i), pState->di_CurrValue));
      SOS_PostEvent(pEvent->em_ProcessId, &msg);

      pState->di_ChangeFlag = false;
    }

    pEvent++;
    pState++;
  }
}

//----------------------------------------------------------------------------

static void updateOutputs(void)
{
  digitalWrite(LED_BUILTIN, ((io_OutputState & PIN_DEBUG_LED) != 0) ? HIGH : LOW);
}

//----------------------------------------------------------------------------

inline static void modifyState(uint16_t bitMask, bool state)
{
  if (state)
  {
    io_OutputState |= bitMask;
  }
  else
  {
    io_OutputState &= ~bitMask;
  }
  updateOutputs();
}

//----------------------------------------------------------------------------

static void configureOutputs(void)
{
  /*
   * Set default values. Note that these are the logical signal states.
   * Inversion for active low outputs takes place in updateOutputs()
   */
  pinMode(LED_BUILTIN, OUTPUT);

  io_OutputState = 0x0000;
  updateOutputs();
}

//----------------------------------------------------------------------------

static void configureInputs(void)
{
  pinMode(PIN_KEY_LEFT_N, INPUT_PULLUP);
  pinMode(PIN_KEY_RIGHT_N, INPUT_PULLUP);
  pinMode(PIN_KEY_MODE_N, INPUT_PULLUP);
}

//--- global functions ------------------------------------------------------

void IO_SetDebugLED(bool state)
{
  modifyState(PIN_DEBUG_LED, state);
}

//----------------------------------------------------------------------------

void IO_ForceStateUpdate(IO_Inputs input)
{
  // D(("Requesting status report of input %s\n",
  //   inputToString( input ) ));
  io_InputState[input].di_ChangeFlag = true;

  reportChanges();
}

//--- OS interface -----------------------------------------------------------

void IO_Tick(void)
{
  /* This function is called once per system tick */

  /* Check digital inputs (debounce, report state change) */

  scanAllInputs();
  reportChanges();
}

//----------------------------------------------------------------------------

void IO_Run(void)
{
  /* Called on every process loop */

  updateOutputs();
}

//----------------------------------------------------------------------------

void IO_Init(void)
{
  IO_DigInState *pState = io_InputState;

  /* Initialize PIC hardware */
  configureInputs();
  configureOutputs();

  /* Initialize Input state registers (assume input is low) */
  for (auto i = 0; i < IO_Inputs_Max; i++)
  {
    pState->di_ShiftReg = 0x00;
    pState->di_ChangeFlag = false;
    pState->di_CurrValue = IO_IN_LOW;
    pState->di_TimeActive = 0;

    pState++;
  }

  /* Load current pin values to get initial state */
  for (auto i = 0; i < 3; i++)
  {
    scanAllInputs();
  }

  /* Clear the changed flag to suppress initial state change reports */
  pState = io_InputState;
  for (auto i = 0; i < IO_Inputs_Max; i++)
  {
    pState->di_ChangeFlag = false;
    pState++;
  }
}

//--- eof --------------------------------------------------------------------
