/*
 * Debugging functions for Rowley CrossWorks...
 * Since we use the same code for both IDEs, we need to create some dummy functions for Eclipse
 * CROSSDEBUG=1 defined for "THUMB Flash Debug" only
 */

#ifndef __openDrive_debug_H
#define __openDrive_debug_H

#ifdef CROSSDEBUG
#include "cross_studio_io.h"
#else
int debug_printf(const char *format, ...);
#endif

#endif /* end __openDrive_debug_H */
