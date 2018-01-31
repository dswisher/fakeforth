## TODO ##

* Virtual Machine
  * On stack underflow, don't exit
  * Rather than implementing PUTC, PSTACK, PUTN, GETC, etc., use syscall-type mechanism
  * On reset, clear input buffers
  * Should arith/logic operators (ADD, SUB, AND, etc) set the zero flag?
  * Put data stack and return stack in memory, then implement `RSP@`, `RSP!`, `DSP@` and `DSP!`
* Forth
  * Add "ok" prompt or something?
  * Implement `CHAR`
  * Implement comparison words (see TODOs in ff.asm)
  * Implement remaining memory words (`+!`, `-!`, etc; see TODOs in ff.asm)
  * Implement remaining built-in constant words: `VERSION`, `R0`, `DOCOL`, `F_IMMED`, `F_LENMASK`
  * Implement `0BRANCH` word
* Debugger:
  * Add sentinal words before and after dictionary definitions and enhance `dict` command to search for these to align
  * In `print`, for CA and IP, show word+offset, such as `QUIT+2`
  * Implement tracepoints (print summary state when line is hit)
  * Show ascii character(s) next to registers, perhaps disassembly (CMP X, 0x005C  .\)
  * Change `pc` and similar commands to just push an "address" on the stack, then implement `@` and `!` so we can update registers
  * In `print`, show symbols that match register values?
  * Implement reverse symbol lookup word (find symbol given address)
  * Implement simple arithmetic
  * Tie source code (via map file?) into debugger and show via `list`
  * Make aliases into aliases ("p" for "print") rather than defining the word multiple times
  * Add a help command to list all the commands
* Assembler:
  * Enhance `.word` so it can take multiple values (`.word FOO, BAR, $26`)
  * BUG: handle `CMP X, $' '`     (thinks the space is between args), also `.ascii "Two Words"`, which segfaults
  * Handle `CMP X, $'\n'`         (backslash escaping)
  * Improve literal handling: "$20" is hex 0x20, "20" is decimal
  * Implement local labels (`1:`, then `1f` and/or `1b`)
  * Add psuedo ops to move between data and code areas
  * Write map (human readable assembled output) to a file - switch to two-pass?
  * If a label is duplicated, issue an error
  * Implement multi-line comments


## DONE ##

* ~~BUG: `BRANCH` is not working correctly, as `ADD IP, (IP)` where `(IP)` is `$-8` is going forward, not back~~
* Virtual Machine
  * ~~Implement unary bitwise opcode: NOT~~
  * ~~Implement binary bitwise opcodes: AND, OR, XOR~~
  * ~~Implement BRK to drop out to debugger~~
  * ~~Implement DCLR and RCLR to clear stacks~~
  * ~~Implement NEG to finish `>NUMBER`~~
  * ~~Implement MUL~~
  * ~~Remove STOS hack and redo load and store opcodes to have both a word flavor and a byte flavor~~
  * ~~Implement SUB opcode~~
  * ~~Need ability to store a word or just a byte (WORD is not working, as high byte overwrites prev char); perhaps store string opcode? STOS?~~
  * ~~Implement conditional jump opcodes: JEQ, JNE, etc.~~
  * ~~Implement CMP opcode - define status flags (carry, zero, sign, etc)~~
  * ~~Implement CALL and RET opcodes - use separate return stack~~
  * ~~REDO opcodes: 6 bytes for op-code, 2 bytes for address mode~~
  * ~~Get rid of GO and replace with JMP with proper address modes~~
* Forth
  * ~~Implement `-`, `*` and `/MOD`~~
  * ~~Implement tick word~~
  * ~~Implement `EXECUTE`~~
  * ~~Implement return-stack words: `>R`, `R>`, `RDROP`~~
  * ~~Change hacky DOT and SDOT to pass the BASE to eliminate dependence on symbols~~
  * ~~Implement remaining "easy" forth primitives (see TODOs in ff.asm)~~
  * ~~Finish `INTERPRET` (compiling not quite working right)~~
  * ~~Implement IMMEDIATE and HIDDEN flags!~~
  * ~~Implement `HIDDEN` (which requires flags on length byte)~~
  * ~~Implement `,` (comma)~~
  * ~~Implement `:` (colon)~~
  * ~~Implement `;` (semi-colon)~~
  * ~~Implement placeholder `.S` for testing~~
  * ~~Implement `>NUMBER` (`NUMBER` in JonesForth?)~~
  * ~~Implement `FIND`~~
  * ~~Implement `WORD`~~
  * ~~Change sample to do 2DUP, which calls DUP, to see if nesting works~~
  * ~~Implement `KEY`, which requires new GETC opcode~~
  * ~~Implement `EMIT`, which requires new PUTC opcode~~
  * ~~Implement `LIT`, `!` and `@`~~
* Debugger:
  * ~~Implement word to list breakpoints~~
  * ~~Implement step-over (execute `CALL` and stop on next line)~~
  * ~~Implement a word to dump a Forth dictionary entry~~
  * ~~Keep track of the last PC and show that one bit of history when disassembling~~
  * ~~BAD IDEA: Do not show symbols for mode 1 addressing~~
  * ~~For readline history, don't add to history if its the same as the last entry (no dups)~~
  * ~~Implement breakpoints and a restart command~~
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
  * ~~BUG: ignore semi-colon inside string so `.dict ";"` will work!~~
  * ~~Add .asciz to embed null-terminated strings~~
  * ~~Write pseudo to create Forth dictionary header~~
  * ~~Write symbols to a file~~
  * ~~If a label is undefined, issue an error~~
  * ~~Add line numbers to assembler errors~~
  * ~~Handle assembler lines with a label but no opcode~~
  * ~~Tweak assembler to take one file name; build other files (output, debug) based on that name~~
* ~~Change Makefile to compile common .c files to .o files once rather than for each binary~~

