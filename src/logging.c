#include "global.h"
#include "logging.h"
#include <stdarg.h>
#include <nitro.h>

// NOTE: DEBUG LOGS ONLY WORK ON DESMUME!
//  - Open Desmume via the terminal. Any log messages will appear in the
//    terminal.

extern void debugsyscall(void *str);


void desmume_printf(const char *format, ...) {
    char buffer[0x800] = {0};
    va_list args;

    va_start(args, format);
    OS_VSPrintf(buffer, format, args);
    va_end(args);

    debugsyscall(buffer);
}
