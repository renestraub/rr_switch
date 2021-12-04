#ifndef _APP_H
#define _APP_H

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
void APP_Process(const SOS_Message *pMsg);

/**
 * Initializes Application module.
 *
 * Must be called before any other function of this module can be used.
 **/
void APP_Init(void);

#endif /* _APP_H */

//--- eof --------------------------------------------------------------------
