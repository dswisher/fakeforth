## TODO ##

* Debugger:
  * Implement word to non-destructively print the data stack
  * Implement word to non-destructively print the return stack
  * Implement word to find a dictionary entry (HOW to put a STRING on the debugger STACK?)
  * Implement a memory dump word (just dump bytes, w/o trying to disassemble; include ascii)
  * Implement a word to dump a Forth dictionary entry
  * Allow labels to be pushed onto the stack; perhaps with a way to quote them (so "next" can be used as a label, too)
    * put STRING on stack, then invoke "lookup" word?!
  * Allow numbers to be pushed onto the stack
  * Implement simple arithmetic
* Assembler:
  * Write pseudo to create Forth dictionary header
  * Write map (human readable assembled output) to a file
  * If a label is duplicated, issue an error
  * Implement multi-line comments
* Change Makefile to compile common .c files to .o files once rather than for each binary


## DONE ##

* Debugger:
  * ~~read symbols and use them~~
  * ~~implement disassembly (list) command~~
  * ~~clean up command handling (use a lookup table or some such)~~
* Assembler:
  * ~~Write symbols to a file~~
  * ~~If a label is undefined, issue an error~~
  * ~~Add line numbers to assembler errors~~
  * ~~Handle assembler lines with a label but no opcode~~
  * ~~Tweak assembler to take one file name; build other files (output, debug) based on that name~~

