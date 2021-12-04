#ifndef _IO_H
#define _IO_H

/** 
 * @file
 * Digital Input/Output Process.
 */

//--- includes ---------------------------------------------------------------

#include <stdint.h>

//--- defines ----------------------------------------------------------------

// Events sent on input change
#define IO_INPUT_ACTIVE (0)
#define IO_INPUT_INACTIVE (1)

/**
 * Digital Inputs
 **/
typedef enum _IO_Inputs
{
  IO_KEY_LEFT = 0,
  IO_KEY_RIGHT_N,
  IO_KEY_MODE_N,

  IO_Inputs_Max
} IO_Inputs;

//--- functions --------------------------------------------------------------

// /**
//  * Sets logical state of output.
//  *
//  * This function sets the logical state of a digital output. The physical
//  * state is computed internally. Outputs with inverted logic (active low)
//  * drive '0' when they are active and '1' otherwise.
//  *
//  * @param  state   IN  True: Output is activated, False: Output is deactivated.
//  **/
// void IO_SetEnIO( Bool state );
// void IO_SetEnLpIo( Bool state );
// void IO_SetEnLpCore( Bool state );
// void IO_SetPwrReset( Bool state );
// void IO_SetPmcPwrOk( Bool state );
// void IO_SetDebugLED( Bool state );

/**
 * Force event report of digital input.
 *
 * This method enforces a check of the current state of the selected input.
 * An event is issued containing the state in message parameter 1.
 *
 * @param  voltage  IN  Input to check.
 **/
void IO_ForceStateUpdate(IO_Inputs input);

/**
 * Tick function for process.
 *
 * This function synchronously at the OS tick rate.
 **/
void IO_Tick(void);

/**
 * Run function for process.
 *
 * This function is called asynchronously whenever the scheduler runs.
 **/
void IO_Run(void);

/**
 * Initializes Input/Output module.
 *
 * Must be called before any other function of this module can be used.
 **/
void IO_Init(void);

#endif /* _IO_H */

//--- eof --------------------------------------------------------------------
