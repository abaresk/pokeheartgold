#include "global.h"
#include "logging.h"
#include "main.h"
#include "poke_overlay.h"
#include "stack_trace.h"
#include "stacktrace_lib.h"

void PrintStacktrace(void) {
  unsigned int n;
  unsigned int count;
  StacktraceEntry entries[32]; // allocate memory for 32 stacktrace entries

  // Print the overlays that are currently loaded.
  ActiveOverlays overlays = GetActiveOverlays(OVY_REGION_MAIN);
  printf("Active overlays: ");
  for (int i = 0; i < overlays.length; i++) {
    printf("overlay_%03d ", overlays.overlays[i]);
  }
  printf("\n");

  OSThread *thread = OS_GetCurrentThread();
  printf("Current thread #%d: stack bottom 0x%08x, stack top (tip) 0x%08x\n", thread->id, thread->stackBottom, thread->stackTop);

  // get the 32 deepest stacktrace entries
  count = Stacktrace(entries, (unsigned short *)MainEnd, 32);

  printf("Stack trace:\n");
  for (n=0; n<count; ++n)
    printf("  0x%08x: %s\n", entries[n].addr, entries[n].name);
}
