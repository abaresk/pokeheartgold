#ifndef POKEHEARTGOLD_LOGGING_H
#define POKEHEARTGOLD_LOGGING_H

#include "global.h"

// NOTE: DEBUG LOGS ONLY WORK ON DESMUME!
//  - Open Desmume via the terminal. Any log messages will appear in the
//    terminal.

#define printf (desmume_printf)

void desmume_printf(const char *format, ...);

#endif //POKEHEARTGOLD_LOGGING_H
