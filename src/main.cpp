//--- includes ---------------------------------------------------------------

#include <Arduino.h>
#include <EEPROM.h>
#include <Servo.h>

#include "simpleos.h"
#include "io.h"

#include "main.h" // own header file

//--- types -------------------------------------------------------------------

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

static state_t stateTeachin(const SOS_Message *pMsg)
{
  static bool on = false;
  state_t new_state = state;

  switch (pMsg->msg_Event)
  {
  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      new_state = OPERATIONAL;

      static const SOS_Message msg1 = {Evt_TickerServo, 0};
      SOS_StartTimer(Timer_Main50, 50, Process_Main, &msg1);
    }
    break;

  case Evt_KeyLeft:
    setServoPos(current_pos + SERVO_STEP);
    break;

  case Evt_KeyRight:
    setServoPos(current_pos - SERVO_STEP);
    break;

  case Evt_TickerLED: // Toggle LED to indicate trach in mode
    on = !on;
    IO_SetDebugLED(on);

    static const SOS_Message msg2 = {Evt_TickerLED, 0};
    SOS_StartTimer(Timer_Main250, 250, Process_Main, &msg2);
    break;

  default:
    break;
  }

  return new_state;
}

static state_t stateOperational(const SOS_Message *pMsg)
{
  state_t new_state = state;

  switch (pMsg->msg_Event)
  {
  case Evt_KeyMode:
    if (pMsg->msg_Param1 == IO_INPUT_ACTIVE)
    {
      static const SOS_Message msg1 = {Evt_TickerLED, 0};
      SOS_StartTimer(Timer_Main250, 250, Process_Main, &msg1);
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
    // Serial.println(String(millis()));
    static const SOS_Message msg = {Evt_TickerServo, 0};
    SOS_StartTimer(Timer_Main50, 50, Process_Main, &msg);

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
      digitalWrite(LED_BUILTIN, HIGH);
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

void MAIN_Process(const SOS_Message *pMsg)
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
    Serial.println("New state" + String(state));
  }
}

void MAIN_Init()
{
  // Try to restore last position from EEPROM
  const uint8_t x = EEPROM.read(0x0000);
  if (x != 0xFF)
  {
    Serial.println(String("Restoring ") + String(x));
    set_pos = x;
  }

  myservo.attach(10);
  // myservo.write(current_pos);
  setServoPos(set_pos);

  static const SOS_Message msg = {Evt_TickerServo, 0};
  SOS_StartTimer(Timer_Main50, 50, Process_Main, &msg);
}

void setup()
{
  Serial.begin(115200);

  TMR_Init(); // Start Timer system
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
