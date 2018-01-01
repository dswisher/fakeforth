## Fake Forth ##

A toy implementation of Forth, using a homebrew simulated machine.


### Assembly Language Notes ###

* Labels - should start with a letter, contain letters, digits, underscore, hyphen; not match a register
* Load addressing modes, using the two bits for mode, for registers a and b,
  * `LOAD a, b` - mode 0 - copy the contents of b into a
  * `LOAD a, $N` - mode 1 - load the value N into a
  * `LOAD a, label` - mode 1 - load the address of the specified label into a
  * `LOAD a, (b)` - mode 2 - load into a the contents of the memory address in b
  * `LOAD a, (label)` - mode 3 - load into a the contents of the memory address of label

