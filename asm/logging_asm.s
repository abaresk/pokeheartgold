	.include "asm/macros.inc"
	.include "logging.inc"
	.include "global.inc"

	.text

	thumb_func_start debugsyscall
debugsyscall:
    swi 0xFC
    bx lr
	thumb_func_end debugsyscall
