#pragma once

/*!
\defgroup		Stacktrace Module

  Stacktrace is an experiment by Peter Schraut (www.console-dev.de) to retrieve a stack-trace without
  the traditional debug information. Stacktrace tries to interpret the stack-pointer (SP) and program-counter (PC)
  by its own, therefor pretty experimental.

  Stacktrace information will be most informative with -mpoke-function-name build configurations.
  Add -mpoke-function-name to "CFLAGS" in your project makefile, to get beside the function address,
  the name of that function as well.
  
  Stacktrace works with thumb code only. You can switch to thumb mode by adding -mthumb to the
  "ARCH" variable in your project makefile.

  The optimization level my tests worked best with is -O2.
  Stacktrace has problems interpreting code generated with optimizations turned off -O0. In my tests
  a few branch opcodes forced Stacktrace to infinite loops, so I added a mechanism to cancel Stacktrace
  when it detects it has visited too many instructions. You still get the trace until this position!
  -O3 and above start to inline code, so the stack-trace is not exactly what you would expect.

  Here is what you need to adjust to your makefile:
    ARCH   := -mthumb -mthumb-interwork
    CFLAGS := -O2 -mpoke-function-name

  Stacktrace is not a works-in-all-ways module. Actually, I wonder that it works so well at all, 
  but getting a stack-trace with a chance of 70% is better than no stack-trace. You are welcome to
  improve and send me your results, you can find my email address and further information to this module at 
  http://www.console-dev.de
*/

#ifdef __cplusplus
  extern "C" {
#endif

/*!
\ingroup		Stacktrace
\brief			Describes one entry in a stack-trace list.
*/
typedef struct
{
  unsigned int addr;  //!< Return address
  const char *name;   //!< Name of function
} StacktraceEntry;

/*!
\ingroup		    Stacktrace
\brief			    Gets a stack-trace

\param[in,out]	dest
  Must point to an allocated buffer of at least "count" elements.
  Results of the stack-trace are stored at this address.

\param[in]		  count
  Specifies how many StacktraceEntry elements are allocated for "dest".
  This controls the back-trace depth as well.

\ret
  Returns number of written StacktraceEntry elements to dest.

\b Example:
\code
void OutputStacktrace()
{
  unsigned int n;
  unsigned int count;
  StacktraceEntry entries[32]; // allocate memory for 32 stacktrace entries

  // get only the 10 deepest stacktrace entries
  count = Stacktrace(entries, 10);

  for (n=0; n<count; ++n)
    printf("0x%08x: %s", entries[n].addr, entries[n].name);
}
\endcode
*/
unsigned int Stacktrace(StacktraceEntry* dest, unsigned short *mainEnd, unsigned int count);

#ifdef __cplusplus
  }
#endif
