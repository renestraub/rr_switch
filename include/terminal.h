#ifndef _TERMINAL_H
#define _TERMINAL_H

/** 
 * @file
 * Terminal Input/Output Process.
 */

//--- includes ---------------------------------------------------------------

// #include <stdint.h>

//--- defines ----------------------------------------------------------------


//--- types ------------------------------------------------------------------



//--- functions --------------------------------------------------------------

/**
 * Run function for process.
 *
 * This function is called asynchronously whenever the scheduler runs.
 **/
void TERM_Run(void);

/**
 * Initializes Input/Output module.
 *
 * Must be called before any other function of this module can be used.
 **/
void TERM_Init(void);

#endif /* _TERMINAL_H */

//--- eof --------------------------------------------------------------------
