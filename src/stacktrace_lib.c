// Forked from http://www.console-dev.de/project/stack-trace-for-nds

#include <stdarg.h>
#include <string.h>
#include "logging.h"
#include "stacktrace_lib.h"

// NOTE: Only works when compiled in and called from thumb mode.

// use this switch to output lots of debug messages
// during a stack walk. set to 1 to switch to verbose mode.
#define STACKTRACE_VERBOSE  0

typedef enum
{
  // start address of .text section.
  // this is where the program code is located (main memory)
  TEXT_SECTION_START = 0x02000000,

  // end address of .text section
  TEXT_SECTION_END = TEXT_SECTION_START + 0x400000,

  // specifies how many instructions to visit before
  // aborting the stack walk. this is necessary in case
  // we come along infinite loops or the like that this
  // stack walker does not handle correctly. rather than
  // being stuck in this situation we abort the operation
  // along with an error message.
  MAX_VISIT_INSTRUCTIONS = 0x80000,

  // specifies how many instructions to visit when
  // searching backwards for the function name, before
  // giving up.
  MAX_FUNCNAME_VISITS = 0x1000,

} ContantsEnum;

// when a function name could not be resolved, this name is used instead. 
// unresolved names usually occur when not passing -mpoke-function-name
// to the compiler or when using hand written assembler code.
static const char* const UNKNOWN_FUNCITON_NAME= "???";

// when we detect an error during the stack walk we display
// this message as very last StacktraceEntry in the returned list.
// see MAX_VISIT_INSTRUCTIONS for more information.
static const char* const ERROR_DURING_STACKWALK= "Error during stack walk, process aborted";


// prints formatted text to the debug output console.
#define OutputDebugString (printf)

// 
// THUMB.18: unconditional branch
// 
// Opcode Format
// 
//   Bit    Expl.
//   15-11  Must be 11100b for this type of instructions
//   N/A    Opcode (fixed)
//           B label   ;branch (jump)
//   10-0   Signed Offset, step 2 ($+4-2048..$+4+2046)
// 
// Return: No flags affected, PC adjusted.
// Execution Time: 2S+
// 
typedef union __attribute__ ((packed))
{
  struct
  {
    signed   short ofs:11;
    unsigned short id:5;
  };
  unsigned short raw;
} OpcodeUnconditionalBranch;

static const unsigned int OpcodeUnconditionalBranch_Mask = 0xf800;
static const unsigned int OpcodeUnconditionalBranch_Id   = 0xe000;

// 
//  THUMB.13: add offset to stack pointer
// 
// Opcode Format
// 
//   Bit    Expl.
//   15-8   Must be 10110000b (0xB0) for this type of instructions
//   7      Opcode/Sign
//           0: ADD  SP,#nn       ;SP = SP + nn
//           1: ADD  SP,#-nn      ;SP = SP - nn
//   6-0    nn - Unsigned Offset    (0-508, step 4)
// 
// Return: No flags affected, SP adjusted.
// Execution Time: 1S
// 
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned short ofs:7;
    unsigned short neg:1;
    unsigned short id:8;
  };
  unsigned short raw;
} OpcodeAddOfsSP;

static const unsigned short OpcodeAddOfsSP_Mask = 0xff00;
static const unsigned short OpcodeAddOfsSP_Id   = 0xb000;


// I have no idea what official opcode term this actually is.
// Didn't find valuable info in GBATek nor the arm instruction
// quick reference about it.
// If you know what this is, please let me know!
//
//  hex          code     binary representation
// 4485	  add	r13, r0	    010001001 0000 101
// 448D	  add	r13, r1	    010001001 0001 101
// 4495	  add	r13, r2	    010001001 0010 101
// 449D	  add	r13, r3	    010001001 0011 101
// 44a5	  add	r13, r4	    010001001 0100 101
// 44ad	  add	r13, r5	    010001001 0101 101
// 44b5	  add	r13, r6	    010001001 0110 101
// 44bd	  add	r13, r7	    010001001 0111 101
//
// The upper 9bits seem to be 010001001 and the lower 3bits 101 always.
// I think bits 4..7 represent the register rn.
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned short dunno:3;
    unsigned short rn:4;
    unsigned short id:9;
  };
  unsigned short raw;
} OpcodeAddRegSP;

static const unsigned short OpcodeAddRegSP_Mask = 0x4487; // 111111111 0000 111
static const unsigned short OpcodeAddRegSP_Id   = 0x4485; // 010001001 0000 101

//
//  THUMB.14: push/pop registers
// 
// Opcode Format
// 
//   Bit    Expl.
//   15-12  Must be 1011b for this type of instructions
//   11     Opcode (0-1)
//           0: PUSH {rlist}{LR}   ;store in memory, decrements SP (R13)
//           1: POP  {rlist}{PC}   ;load from memory, increments SP (R13)
//   10-9   Must be 10b for this type of instructions
//   8      PC/LR Bit (0-1)
//           0: No
//           1: PUSH LR (R14), or POP PC (R15)
//   7-0    rlist - List of Registers (R7..R0)
// 
// In THUMB mode stack is always meant to be 'full descending', ie. PUSH is equivalent to 'STMFD/STMDB' and POP to 'LDMFD/LDMIA' in ARM mode.
//
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned short rlist:8;
    unsigned short pc:1;
    unsigned short id:7;
  };
  unsigned short raw;
} OpcodePop;

static const unsigned short OpcodePop_Mask = 0xfe00;
static const unsigned short OpcodePop_Id   = 0xbc00;

// 
//  THUMB.6: load PC-relative
// 
// Opcode Format
// 
//   Bit    Expl.
//   15-11  Must be 01001b for this type of instructions
//   N/A    Opcode (fixed)
//            LDR Rd,[PC,#nn]      ;load 32bit    Rd = WORD[PC+nn]
//   10-8   Rd - Destination Register   (R0..R7)
//   7-0    nn - Unsigned offset        (0-1020 in steps of 4)
// 
// The value of PC will be interpreted as (($+4) AND NOT 2).
// Return: No flags affected, data loaded into Rd.
// Execution Time: 1S+1N+1I
//
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned short ofs:8;
    unsigned short rd:3;
    unsigned short code:1;
    unsigned short id:4;
  };
  unsigned short raw;
} OpcodeLdrPcRelative;

static const unsigned short OpcodeLdrPcRelative_Mask = 0xf800;
static const unsigned short OpcodeLdrPcRelative_Id   = 0x4800;

// 
// THUMB.18: branch and exchange
// 
// Opcode Format
// 
//   Bit    Expl.
//   15-10  Must be 010001b for this type of instructions
//   9-8    Opcode (fixed to 11b)
//   7      H1
//           0: BX Rs
//           1: BLX Rs
//   6      H2
//           0: BX Rs     ;branch to address in register Rs (from r0-r7)
//           1: BX Hs     ;branch to address in register Hs (from r8-r15)
//   5-3    Rs/Hs - Source Register  (R0..R7) / (R8..R15) - computed as (R{Rs}) / (R{8 + Rs})
//   2-0    Rd/Hd (not applicable -- fixed to 000b)
// 
// Return: No flags affected, PC adjusted.
// 
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned   short rd:3;
    unsigned   short rs:3;
    unsigned   short h2:1;
    unsigned   short h1:1;
    // opcode merged with id
    unsigned   short id:8;
  };
  unsigned short raw;
} OpcodeBranchAndExchange;

static const unsigned int OpcodeBranchAndExchange_Mask = 0xff00;
static const unsigned int OpcodeBranchAndExchange_Id   = 0x4700;


//  This represents the TAG inserted by the compiler
//  when passing -mpoke-function-name as CFLAGS.
typedef union __attribute__ ((packed))
{
  struct
  {
    unsigned int len:8;
    unsigned int id:24;
  };
  unsigned int raw;
} OpcodeFunctionnameTag;

static const unsigned int OpcodeFunctionnameTag_Mask = 0xffffff00;
static const unsigned int OpcodeFunctionnameTag_Id   = 0x00ff0000;


// Gets the absolute address where the branch points to.
// We assume op is where PC is located atm.
static unsigned short* OpcodeUnconditionalBranch_GetTargetAddr(const OpcodeUnconditionalBranch* op)
{
  unsigned int targetAddr = (unsigned int)(op + op->ofs) + 4;
  targetAddr = targetAddr & ~1; // make sure address is 2byte aligned
  return (unsigned short*)targetAddr;
}

// Gets the absolute address where the opcode points to.
// We assume op is where PC is located atm.
static int* OpcodeLdrPcRelative_GetTargetAddr(const OpcodeLdrPcRelative* op)
{
  // op->ofs is specified in 4byte steps. since op is a 16bit type
  // we multiply op->ofs*2 to get a 4byte offset.
  unsigned int targetAddr = (unsigned int)(op + op->ofs*2) + 4;
  targetAddr = targetAddr & ~3; // make sure address is 4byte aligned
  return (int*)targetAddr;
}

// Gets amount of bits set in "bits"
static unsigned int CountBits(unsigned int bits)
{
  unsigned int count=0;

  while(bits)
  {
    bits &= bits-1;
    ++count;
  }

  return count;
}

static unsigned short *AlignPC(unsigned short *pc) {
  // TODO: figure out why this happens
  if (((unsigned int)pc) & 1)
    return (unsigned short*)((((unsigned int)pc) - 1) & ~1);
  return pc;
}

// Searches for the -mpoke-function-name tag.
// Returns a pointer to the tag on success, otherwise NULL.
static const OpcodeFunctionnameTag *GetFunctionnameTag(unsigned int addr)
{
  const OpcodeFunctionnameTag *tag = (OpcodeFunctionnameTag*)(addr & ~3); // 4byte align address
  unsigned int range=MAX_FUNCNAME_VISITS;

  while(tag->id != OpcodeFunctionnameTag_Id)
  {
    --tag;
    if(--range <= 0)
      return NULL;
  }

  return tag;
}

// Prints content of an "unconditional branch" opcode
// b label
static void PrintOpcodeUnconditionalBranch(const OpcodeUnconditionalBranch* op)
{
  OutputDebugString("OpcodeUnconditionalBranch\n");
  OutputDebugString("  ofs: %d hwords (%d bytes)\n", op->ofs, op->ofs*sizeof(unsigned short));
  OutputDebugString("  TargetAddr: 0x%08x\n", OpcodeUnconditionalBranch_GetTargetAddr(op));
}

static void PrintOpcodeAddRegSP(const OpcodeAddRegSP* op)
{
  OutputDebugString("OpcodeAddRegSP\n");
  OutputDebugString("  rn: %d\n", op->rn);
}

static void PrintOpcodeLdrPcRelative(const OpcodeLdrPcRelative* op)
{
  OutputDebugString("OpcodeLdrPcRelative\n");
  OutputDebugString("  ofs: %d\n", op->ofs);
  OutputDebugString("  rd: %d\n", op->rd);

  int value = *OpcodeLdrPcRelative_GetTargetAddr(op);
  OutputDebugString("  TargetAddr: 0x%08x\n", OpcodeLdrPcRelative_GetTargetAddr(op));
  OutputDebugString("  value: %d (0x%x)\n", value, value);
}

static void PrintOpcodeBranchAndExchange(const OpcodeBranchAndExchange* op)
{
  OutputDebugString("OpcodeBranchAndExchange\n");
  int branchRegister = op->rs + ((op->h2 & 1) ? 8 : 0);
  OutputDebugString("  bx r%d\n", branchRegister);
}

// Prints content of an "add offset to stack pointer" opcode
// add sp, #10
// add sp, #-10
static void PrintOpcodeAddOfsSP(const OpcodeAddOfsSP* op)
{
  OutputDebugString("OpcodeAddOfsSP\n");
  OutputDebugString("  neg: %d\n", op->neg);
  OutputDebugString("  ofs: %d words (0x%x bytes)\n", op->ofs, op->ofs*4);
}

// Prints content of a "pop" opcode
// pop {r0, pc}
static void PrintOpcodePop(const OpcodePop* op)
{
  static const char* const REGS[]= { "r0, ", "r1, ", "r2, ", "r3, ", "r4, ", "r5, ", "r6, ", "r7, " };
  char regsBuffer[64]={0};
  char* regs = regsBuffer;
  unsigned int bit = 0;
  while (bit < 8)
  {
    if((1<<bit) & op->rlist)
    {
      strcpy(regs, REGS[bit]);
      regs += 4;
    }
    ++bit;
  }

  if(op->pc)
    strcpy(regs, "pc");

  OutputDebugString("OpcodePop\n");
  OutputDebugString("  Registers: %s\n", regsBuffer);
}

// Prints content of a function name tag
// See -mpoke-function-name in GCC compiler manual
static void PrintOpcodeFunctionnameTag(const OpcodeFunctionnameTag* op)
{
  const char* const name = (char*)(((unsigned int)op) - op->len);

  OutputDebugString("OpcodeFunctionnameTag\n");
  OutputDebugString("  Name: %s\n", name);
  OutputDebugString("  Length: %d\n", op->len);
  OutputDebugString("  FuncAddr: 0x%08x\n", sizeof(OpcodeFunctionnameTag)+(unsigned int)op);
}

static void PrintStackContents(unsigned int *sp) {
  unsigned int stackBytes = 0x200;

  OutputDebugString("Stack contents:\n");
  for (int i = 0; i < stackBytes; i++) {
    OutputDebugString("sp=0x%08x, sp[%d]=0x%08x\n", (unsigned int *)(sp + i), i, sp[i]);
  }
}



// This function tries to back-trace function calls.
unsigned int Stacktrace(StacktraceEntry* dest, unsigned short *mainEnd, unsigned int count)
{
  unsigned int    *sp=0;   // represents stack pointer
  unsigned short  *pc=0;   // represents program counter
  unsigned int    depth=0; // current back-trace depth (each function increases this by one)
  unsigned int    instructionsVisited=0; // how many instructions we have visited during a stack walk

  if(count == 0 || dest == NULL)
    return 0;

  // get current program counter value (PC)
  __asm volatile ("mov %0, pc \n\t" : "=r"(pc));

  // get current stack pointer value (SP)
  __asm volatile ("mov %0, sp \n\t" : "=r"(sp));

#if STACKTRACE_VERBOSE
  PrintStackContents(sp);
#endif

  // now forward advance the program counter and look for the following instructions:
  //
  //  pop {...pc} : we will adjust PC to the pop'ed value (OpcodePop)
  //  add sp, nn  : we will adjust SP by nn (OpcodeAddOfsSP)
  //  sub sp, nn  : we will adjust SP by nn (OpcodeAddOfsSP)
  //  b   ofs     : will set PC to the addr PC+ofs (OpcodeUnconditionalBranch)
  //  add sp, rn  :  we will adjust SP by the value rn points to (OpcodeLdrPcRelative)
  //
  while(pc > (unsigned short*)TEXT_SECTION_START && pc < (unsigned short*)TEXT_SECTION_END && depth < count && instructionsVisited < MAX_VISIT_INSTRUCTIONS)
  {
#if STACKTRACE_VERBOSE
    OutputDebugString("pc=0x%08x: op=0x%02x, sp=0x%08x, sp[0]=0x%08x\n", pc, *pc, sp, sp[0]);
#endif

    // Since we cannot yet embed function names into the binary, break out once
    // the program counter enters the main function.
    if (mainEnd != NULL && pc < mainEnd) {
      StacktraceEntry* entry = &dest[depth-1]; // overwrite last entry
      entry->addr   = (unsigned int)pc;
      entry->name   = "NitroMain";
      break;
    }

    const OpcodeBranchAndExchange *opBx = (OpcodeBranchAndExchange*)pc;

    if((((OpcodePop*)pc)->raw & OpcodePop_Mask) == OpcodePop_Id)
    {
      const OpcodePop *op = (OpcodePop*)pc;
#if STACKTRACE_VERBOSE
      PrintOpcodePop(op);
#endif

      // get how many registers are being pop'ed.
      // each bit represents one register (up to 8 regs (r0..r7))
      int numregs = CountBits(op->rlist);
      if(op->pc)
        numregs++; // when PC is being pop'ed too, we have one further register

      // increment stack-pointer by the number of registers
      // (we have pop'ed those regs from the stack so we have to advance it)
      sp += numregs;

      // if PC is in the register list
      // we must adjust PC to the value we just pop'ed from stack
      if(op->pc)
      {
        // get PC from stack, fixing alignment if necessary
        pc = AlignPC((unsigned short *)sp[-1]);

        if(pc != NULL)
        {
          // PC is now located in the caller function, try to get its function tag (-mpoke-function-name)
          const OpcodeFunctionnameTag* funcNameTag = GetFunctionnameTag((unsigned int)pc);
#if STACKTRACE_VERBOSE
          PrintOpcodeFunctionnameTag(funcNameTag);
#endif

          // write the StacktraceEntry
          StacktraceEntry* entry = &dest[depth++];
          entry->addr   = (unsigned int)pc;
          // NOTE: Cannot extract function name right now. Metrowerks compiler
          // does not support flag -mpoke-function-name.
          entry->name   = funcNameTag != NULL ? (char*)(((unsigned int)funcNameTag) - funcNameTag->len) : UNKNOWN_FUNCITON_NAME;

          // check if this is the application entry point.
          // yeah, pretty dumb method but it seems to work.
          if(strcmp(entry->name, "NitroMain")==0)
            break;
        }
       }
      else
        ++pc;
    }
    else if((((OpcodeAddRegSP*)pc)->raw & OpcodeAddRegSP_Mask) == OpcodeAddRegSP_Id)
    {
      const OpcodeAddRegSP *op = (OpcodeAddRegSP*)pc;
#if STACKTRACE_VERBOSE
      PrintOpcodeAddRegSP(op);
#endif
      
      // Check if the previous instruction is a load.
      // Usually the offset value is being loaded to the operand
      // right before the "add sp, rn" instruction.
      const OpcodeLdrPcRelative *opLdr = (OpcodeLdrPcRelative*)(op-1);
      if((opLdr->raw & OpcodeLdrPcRelative_Mask) == OpcodeLdrPcRelative_Id)
      {
#if STACKTRACE_VERBOSE
        PrintOpcodeLdrPcRelative(opLdr);
#endif
        // load value
        int value = *OpcodeLdrPcRelative_GetTargetAddr(opLdr);

        // add value to SP. Value is specified in bytes, but SP is
        // a 32bit pointer to we must convert value to 32bit words.
        sp = sp + (value/sizeof(unsigned int));
      }

      ++pc;
    }
    else if((opBx->raw & OpcodeBranchAndExchange_Mask) == OpcodeBranchAndExchange_Id && opBx->h1 == 0)
    {
#if STACKTRACE_VERBOSE
        PrintOpcodeBranchAndExchange(opBx);
#endif
      // NOTE: No need to worry about `bx lr` as this only happens in leaf
      // functions.

      // Go to the address in the link register.
      pc = AlignPC((unsigned short*)sp[-1]);

      // PC is now located in the caller function, try to get its function tag (-mpoke-function-name)
      const OpcodeFunctionnameTag* funcNameTag = GetFunctionnameTag((unsigned int)pc);
#if STACKTRACE_VERBOSE
      PrintOpcodeFunctionnameTag(funcNameTag);
#endif

      // write the StacktraceEntry
      StacktraceEntry* entry = &dest[depth++];
      entry->addr   = (unsigned int)pc;
      entry->name   = funcNameTag != NULL ? (char*)(((unsigned int)funcNameTag) - funcNameTag->len) : UNKNOWN_FUNCITON_NAME;
    }
    else if((((OpcodeAddOfsSP*)pc)->raw & OpcodeAddOfsSP_Mask) == OpcodeAddOfsSP_Id)
    {
      const OpcodeAddOfsSP *op = (OpcodeAddOfsSP*)pc;
#if STACKTRACE_VERBOSE
      PrintOpcodeAddOfsSP(op);
#endif

      // Check if the next instruction is `bx Rs`.
      // If so, extract the next program counter address from the stack before
      // updating the stack pointer.
      const OpcodeBranchAndExchange *opBx = (OpcodeBranchAndExchange*)(op+1);
      if ((opBx->raw & OpcodeBranchAndExchange_Mask) == OpcodeBranchAndExchange_Id) {
        pc = AlignPC((unsigned short*)sp[-1]);

        // PC is now located in the caller function, try to get its function tag (-mpoke-function-name)
        const OpcodeFunctionnameTag* funcNameTag = GetFunctionnameTag((unsigned int)pc);
  #if STACKTRACE_VERBOSE
        PrintOpcodeFunctionnameTag(funcNameTag);
  #endif

        // write the StacktraceEntry
        StacktraceEntry* entry = &dest[depth++];
        entry->addr   = (unsigned int)pc;
        entry->name   = funcNameTag != NULL ? (char*)(((unsigned int)funcNameTag) - funcNameTag->len) : UNKNOWN_FUNCITON_NAME;
      } else {
        ++pc;
      }

      if(op->neg)
        sp = sp - op->ofs;
      else
        sp = sp + op->ofs;
    }
    // This causes an infinite loop where a conditional branch is required to
    // break out of a cycle of unconditional branches.
    // 
    // Just remove it, since Metrowerks C compiler is unlikely to use an
    // unconditional branch to move backwards to a `pop`.
//     else if((((OpcodeUnconditionalBranch*)pc)->raw & OpcodeUnconditionalBranch_Mask) == OpcodeUnconditionalBranch_Id)
//     {
//       const OpcodeUnconditionalBranch *op = (OpcodeUnconditionalBranch*)pc;
// #if STACKTRACE_VERBOSE
//       PrintOpcodeUnconditionalBranch(op);
// #endif

//       pc = OpcodeUnconditionalBranch_GetTargetAddr(op);
//     }
    else
    {
      // whatever instruction this is, just skip it
      ++pc;
    }

    ++instructionsVisited;
  }

  if(instructionsVisited >= MAX_VISIT_INSTRUCTIONS)
  {
    // it is very likely that the stack walker could
    // not successfully back-trace function calls
    // when we are here. set up a dummy entry with
    // the appropriate message.

    if(depth >= count)
      depth = count-1; // overwrite last entry

    StacktraceEntry* entry = &dest[depth++];
    entry->addr   = 0;
    entry->name   = ERROR_DURING_STACKWALK;
  }

  return depth;
}
