#ifndef _MAIN_H
#define _MAIN_H

//--- includes ---------------------------------------------------------------

#include "simpleos.h" // SOS_Message

//--- functions --------------------------------------------------------------

/**
 * Main Process Function
 * 
 * Runs application state machine
 * 
 * @param pMsg Message to process (event, parameter)
 */
void MAIN_Process(const SOS_Message *pMsg);

/**
 * Initializes Input/Output module.
 *
 * Must be called before any other function of this module can be used.
 **/
void MAIN_Init(void);

#endif /* _MAIN_H */

//--- eof --------------------------------------------------------------------
