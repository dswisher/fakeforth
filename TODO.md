## TODO ##

* Debugger:
  * read symbols and use them
  * implement disassembly (list) command
  * clean up command handling (use a lookup table or some such)
* Assembler:
  * Write map (human readable assembled output) to a file
  * If a label is duplicated, issue an error
  * Implement multi-line comments
* Change Makefile to compile common .c files to .o files once rather than for each binary


## DONE ##

* Assembler:
  * ~~Write symbols to a file~~
  * ~~If a label is undefined, issue an error~~
  * ~~Add line numbers to assembler errors~~
  * ~~Handle assembler lines with a label but no opcode~~
  * ~~Tweak assembler to take one file name; build other files (output, debug) based on that name~~

