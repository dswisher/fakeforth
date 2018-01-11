## Fake Forth ##

A toy implementation of Forth, using a homebrew simulated machine.

* Inspiration: [Jones Forth](https://github.com/AlexandreAbreu/jonesforth), [pijFORTHos](https://github.com/organix/pijFORTHos)
* Goal? [Forth Standard](https://forth-standard.org/)


### Simulated Assembly Language Notes ###

* Labels - should start with a letter, contain letters, digits, underscore, hyphen; not match a register
* LOAD addressing modes, using two bits for mode. Given registers a and b:
  * `LOAD a, b` - mode 0 - copy the contents of b into a
  * `LOAD a, $N` - mode 1 - load the value N into a
  * `LOAD a, label` - mode 1 - load the address of the specified label into a
  * `LOAD a, (b)` - mode 2 - load into a the contents of the memory word pointed to by b
  * `LOAD a, (label)` - mode 3 - load into a the contents of the memory word at label
  * `LOAD a, ($N)` - mode 3 - load into a the contents of the memory word at N
* STORE addressing modes, using two bits for mode. Given registers a and b:
  * `STORE a, b` - mode 0 - copy the contents of a into b
  * `STORE a, (b)` - mode 2 - store the contents of a into the memory word pointed to by b
  * `STORE a, (label)` - mode 3 - store the contents of a into the memory word at label
  * `STORE a, ($N)` - mode 3 - store the contents of a into the memory word at N
  * Note that mode 1 does not make sense and is not supported.
* JMP addressing modes, using two bits for mode. Given register a:
  * `JMP a` - mode 0 - jump to the address contained in a
  * `JMP $N` - mode 1 - jump to address N
  * `JMP (a)` - mode 2 - jump to the address contained in the memory word pointed to by a
  * `JUMP ($N)` - mode 3 - jump to the address contained in the memory pointed to by N
* CMP addressing modes, using two bits for mode. Given registers a and b:
  * `CMP a, b` - mode 0 - compare registers a and b
  * `CMP a, $N` - mode 1 - compare register a to the literal value N
  * `CMP a, (b)` - mode 2 - compare a to the memory word pointed to by b
  * `CMP a, ($N)` - mode 3 - compare a to the memory word pointed to by the memory word N
* CALL - push address of next opcode on call stack, jump to specified address
* RET - pop address off call stack


## Possibly Useful Links

* Assembly language: [x86](https://en.wikibooks.org/wiki/X86_Assembly) - [ARM](http://www.davespace.co.uk/arm/introduction-to-arm/index.html)
* Writing an assembler: [Webster](http://plantation-productions.com/Webster/RollYourOwn/index.html)
* Emulators: [wikipedia list](https://en.wikipedia.org/wiki/List_of_computer_system_emulators) - [QEMU](https://www.qemu.org/)

