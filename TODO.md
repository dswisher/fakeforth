## TODO ##

* Virtual Machine
  * Implement conditional jump opcodes: JEQ, JNE, etc.
  * Implement local labels (`1:`, then `1f` and/or `1b`)
* Forth
  * Implement `WORD` - needs compare and conditional jump opcodes
  * Implement `INTERPRET`
* Debugger:
  * Change `pc` and similar commands to just push an "address" on the stack, then implement `@` and `!` so we can update registers
  * Implement breakpoints and a restart command
  * Keep track of the last PC and show that one bit of history when disassembling
  * In `print`, show symbols that match register values
  * Implement reverse symbol lookup word (find symbol given address)
  * Implement a word to dump a Forth dictionary entry
  * Implement simple arithmetic
  * Tie source code (via map file?) into debugger and show via `list`
  * Make aliases into aliases ("p" for "print") rather than defining the word multiple times
* Assembler:
  * Add psuedo ops to move between data and code areas
  * Write map (human readable assembled output) to a file - switch to two-pass?
  * If a label is duplicated, issue an error
  * Implement multi-line comments


## DONE ##

* Virtual Machine
  * ~~Implement CMP opcode - define status flags (carry, zero, sign, etc)~~
  * ~~Implement CALL and RET opcodes - use separate return stack~~
  * ~~REDO opcodes: 6 bytes for op-code, 2 bytes for address mode~~
  * ~~Get rid of GO and replace with JMP with proper address modes~~
* Forth
  * ~~Change sample to do 2DUP, which calls DUP, to see if nesting works~~
  * ~~Implement `KEY`, which requires new GETC opcode~~
  * ~~Implement `EMIT`, which requires new PUTC opcode~~
  * ~~Implement `LIT`, `!` and `@`~~
* Debugger:
  * ~~Implement word to find a dictionary entry given a string on the stack~~
  * ~~Allow labels to be pushed onto the stack; perhaps with a way to quote them (so "next" can be used as a label, too)~~
    * ~~put STRING on stack, then invoke "lookup" word?!~~
  * ~~Implement a memory dump word (just dump bytes, w/o trying to disassemble; include ascii)~~
  * ~~Allow numbers to be pushed onto the stack~~
  * ~~Implement word to non-destructively print the data stack~~
  * ~~Implement word to non-destructively print the return stack~~
  * ~~Improve readline integration~~
    * ~~Save/restore history~~
    * ~~Links: [docs](http://www.delorie.com/gnu/docs/readline/rlman_23.html) - [hist](https://tiswww.cwru.edu/php/chet/readline/history.html#SEC10) - [sample1](https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/)~~
  * ~~read symbols and use them~~
  * ~~implement disassembly (list) command~~
  * ~~clean up command handling (use a lookup table or some such)~~
* Assembler:
  * ~~Write pseudo to create Forth dictionary header~~
  * ~~Write symbols to a file~~
  * ~~If a label is undefined, issue an error~~
  * ~~Add line numbers to assembler errors~~
  * ~~Handle assembler lines with a label but no opcode~~
  * ~~Tweak assembler to take one file name; build other files (output, debug) based on that name~~
* ~~Change Makefile to compile common .c files to .o files once rather than for each binary~~

