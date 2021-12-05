//--- includes ---------------------------------------------------------------

#include <Arduino.h>
#include <EEPROM.h>
#include <Servo.h>

#include "simpleos.h"
#include "io.h"

#include "app.h" // own header file

//--- types ------------------------------------------------------------------

enum state_t
{
  OPERATIONAL,
  TEACHIN
};

//--- constants --------------------------------------------------------------

static const int SERVO_STEP = 1;
static const int SERVO_STEP_RUN = 3;
static const int SERVO_MAX_ANGLE = 45;

//--- variables --------------------------------------------------------------

static state_t state = OPERATIONAL;
static Servo myservo;
static int current_pos = 90;
static int set_pos = 90;

//--- local functions --------------------------------------------------------

static void setServoPos(int pos)
{
  current_pos = constrain(pos, SERVO_MAX_ANGLE, 180 - SERVO_MAX_ANGLE);
  myservo.write(current_pos);
}

//----------------------------------------------------------------------------

static void teachinEnter()
{
  // Serial.println("teachinEnter");

  setServoPos(90);

  static const SOS_Message msg = {Evt_TickerLED, 0};
  SOS_StartTimer(Timer_Main250, 250, Process_Main, msg);
}

static state_t stateTeachin(const SOS_Message *pMsg)
{
  static bool on = false;
  state_t new_state = state;

  switch (pMsg->msg_Event)
  {
  case Evt_Enter:
    teachinEnter();
    break;

  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      new_state = OPERATIONAL;
    }
    break;

  case Evt_KeyLeft:
    setServoPos(current_pos - SERVO_STEP);
    break;

  case Evt_KeyRight:
    setServoPos(current_pos + SERVO_STEP);
    break;

  case Evt_TickerLED: // Toggle LED to indicate teach-in mode
    on = !on;
    IO_SetDebugLED(on);
    SOS_RestartTimer(Timer_Main250, 250);
    break;

  default:
    break;
  }

  return new_state;
}

//----------------------------------------------------------------------------

static void operationalEnter()
{
  // Serial.println("operationalEnter");

  static const SOS_Message msg = {Evt_TickerServo, 0};
  SOS_StartTimer(Timer_Main50, 50, Process_Main, msg);
}

static state_t stateOperational(const SOS_Message *pMsg)
{
  state_t new_state = state;

  switch (pMsg->msg_Event)
  {
  case Evt_Enter:
    operationalEnter();
    break;

  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      new_state = TEACHIN;
    }
    break;

  case Evt_KeyLeft:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      set_pos = SERVO_MAX_ANGLE;
      Serial.println("Setting " + String(set_pos));
      EEPROM.update(0x0000, set_pos);
    }
    break;

  case Evt_KeyRight:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      set_pos = 180 - SERVO_MAX_ANGLE;
      Serial.println("Setting " + String(set_pos));
      EEPROM.update(0x0000, set_pos);
    }
    break;

  case Evt_TickerServo:
  {
    SOS_RestartTimer(Timer_Main50, 50);

    auto old_pos = current_pos;
    if (current_pos < set_pos)
    {
      current_pos += SERVO_STEP_RUN;
    }
    else if (current_pos > set_pos)
    {
      current_pos -= SERVO_STEP_RUN;
    }

    if (current_pos != old_pos)
    {
      setServoPos(current_pos);
      IO_SetDebugLED(true);
      Serial.println("pos change to " + String(current_pos));
    }
    else
    {
      IO_SetDebugLED(false);
    }
  }

  default:
    break;
  }

  return new_state;
}

//--- global functions -------------------------------------------------------

void APP_Process(const SOS_Message *pMsg)
{
  state_t new_state;

  switch (state)
  {
  case OPERATIONAL:
    new_state = stateOperational(pMsg);
    break;

  case TEACHIN:
    new_state = stateTeachin(pMsg);
    break;

  default:
    break;
  }

  if (new_state != state)
  {
    state = new_state;
    Serial.println("New state " + String(state));

    // Post Enter message for new state
    SOS_PostEventArgs(Process_Main, Evt_Enter, 0);
  }
}

//----------------------------------------------------------------------------

void APP_Init()
{
  // Try to restore last position from EEPROM
  const uint8_t x = EEPROM.read(0x0000);
  if (x != 0xFF)
  {
    Serial.println(String("Restoring position ") + String(x));
    set_pos = x;
  }

  myservo.attach(10);
  setServoPos(set_pos);

  // Start state machine
  SOS_PostEventArgs(Process_Main, Evt_Enter, 0);
}

//--- eof --------------------------------------------------------------------
