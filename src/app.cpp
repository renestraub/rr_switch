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

enum position_t
{
  UNKNOWN,
  LEFT,
  RIGHT
};

//--- constants --------------------------------------------------------------

static const uint8_t MAGIC_TOKEN = 'R';

static const int SERVO_STEP = 1;
static const int SERVO_STEP_RUN = 1;

// Absolute limit for servo movement in programming mode
static const int SERVO_MAX_ANGLE = 20;
static const int SERVE_MAX_ANGLE_LEFT = 90 - SERVO_MAX_ANGLE;
static const int SERVE_MAX_ANGLE_RIGHT = 90 + SERVO_MAX_ANGLE;

//--- variables --------------------------------------------------------------

static state_t state = OPERATIONAL;
static Servo myservo;

// Servo position for switch left/right position. Can be modified in
// programming mode
static int left_pos = 80;
static int right_pos = 100;

static position_t switch_pos = UNKNOWN;
static int current_pos = 90;
static int set_pos = 90;

//--- local functions --------------------------------------------------------

static void setServoPos(int pos)
{
  current_pos = constrain(pos, left_pos, right_pos);
  myservo.write(current_pos);
}

static void setServoPosUnlimited(int pos)
{
  current_pos = constrain(pos, SERVE_MAX_ANGLE_LEFT, SERVE_MAX_ANGLE_RIGHT);
  myservo.write(current_pos);
}

//----------------------------------------------------------------------------

static void showTeachinMenu()
{
  Serial.println("Programming menu");
  Serial.println(" h: show this help");
  Serial.println(" l: move left");
  Serial.println(" r: move right");
  Serial.println(" x: Leave mode");
}

static void moveLeft()
{
  int new_pos = current_pos - SERVO_STEP;
  Serial.println("Moving limit to " + String(new_pos));
  setServoPosUnlimited(new_pos);
}

static void moveRight()
{
  int new_pos = current_pos + SERVO_STEP;
  Serial.println("Moving limit to " + String(new_pos));
  setServoPosUnlimited(new_pos);
}

static void teachinEnter()
{
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
    showTeachinMenu();
    break;

  case Evt_Char:
    if (pMsg->msg_Param1 == 'h')
    {
      showTeachinMenu();
    }
    else if (pMsg->msg_Param1 == 'x')
    {
      // TODO: Save settings to EEPROM
      bool save = false;

      switch (switch_pos)
      {
      case LEFT:
        Serial.println("Updating left limits");
        left_pos = set_pos = current_pos;
        save = true;
        break;

      case RIGHT:
        Serial.println("Updating right limits");
        right_pos = set_pos = current_pos;
        save = true;
        break;

      case UNKNOWN:
      default:
        break;
      }

      if (save)
      {
        Serial.println(String("Saving limits ") + String(left_pos) + "," + String(right_pos));

        // TODO: refactor to settings functions

        EEPROM.write(0x0009, left_pos);
        EEPROM.write(0x000A, right_pos);
        EEPROM.write(0x0008, MAGIC_TOKEN);
      }

      new_state = OPERATIONAL;
    }
    else if (pMsg->msg_Param1 == 'l')
    {
      moveLeft();
    }
    else if (pMsg->msg_Param1 == 'r')
    {
      moveRight();
    }

  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      new_state = OPERATIONAL;
    }
    break;

  case Evt_KeyLeft:
    moveLeft();
    break;

  case Evt_KeyRight:
    moveRight();
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

static void showMainMenu()
{
  Serial.println("Programming menu");
  Serial.println(" h: show this help");
  Serial.println(" l: switch left");
  Serial.println(" r: switch right");
  Serial.println(" p: programming mode");
}

static void operationalEnter()
{
  // Serial.println("operationalEnter");

  static const SOS_Message msg = {Evt_TickerServo, 0};
  SOS_StartTimer(Timer_Main50, 50, Process_Main, msg);
}

static void switchLeft()
{
  switch_pos = LEFT;
  set_pos = left_pos;
  Serial.println("Switching to left " + String(set_pos));
  EEPROM.update(0x0000, set_pos);
}

static void switchRight()
{
  switch_pos = RIGHT;
  set_pos = right_pos;
  Serial.println("Switching to right " + String(set_pos));
  EEPROM.update(0x0000, set_pos);
}

static state_t stateOperational(const SOS_Message *pMsg)
{
  state_t new_state = state;

  switch (pMsg->msg_Event)
  {
  case Evt_Enter:
    operationalEnter();
    break;

  case Evt_Char:
    if (pMsg->msg_Param1 == 'h')
    {
      showMainMenu();
    }
    else if (pMsg->msg_Param1 == 'p')
    {
      new_state = TEACHIN;
    }
    else if (pMsg->msg_Param1 == 'l')
    {
      switchLeft();
    }
    else if (pMsg->msg_Param1 == 'r')
    {
      switchRight();
    }

  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      new_state = TEACHIN;
    }
    break;

  case Evt_KeyLeft:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      switchLeft();
    }
    break;

  case Evt_KeyRight:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      switchRight();
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

    /* Move servo until we reach final position */
    if (current_pos != old_pos)
    {
      setServoPos(current_pos);
      IO_SetDebugLED(true);
      // Serial.println("pos change to " + String(current_pos));
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
  // TODO: refactor to settings functions

  // Try to restore left/right limits from EEPROM
  const uint8_t magic = EEPROM.read(0x0008);
  if (magic == MAGIC_TOKEN)
  {
    const uint8_t left = EEPROM.read(0x0009);
    const uint8_t right = EEPROM.read(0x000A);
    Serial.println(String("Restoring limits ") + String(left) + "," + String(right));

    // TODO: Sanity check --> max limits, left > right or similar
    left_pos = left;
    right_pos = right;
  }
  else
  {
    // No valid EEPROM config, use defaults
    Serial.println("Using default limits");
  }

  // Try to restore last position from EEPROM
  const uint8_t pos = EEPROM.read(0x0000);
  if (pos != 0xFF)
  {
    Serial.println(String("Restoring position ") + String(pos));
    set_pos = pos;
  }
  else
  {
    // No valid EEPROM config, use defaults
    Serial.println("Using default position");
  }

  myservo.attach(10);
  setServoPos(set_pos);

  // Start state machine
  SOS_PostEventArgs(Process_Main, Evt_Enter, 0);
}

//--- eof --------------------------------------------------------------------
