## Fake Forth ##

A toy implementation of Forth, using a homebrew simulated machine.

* Inspiration: [Jones Forth](https://github.com/AlexandreAbreu/jonesforth)
* Goal? [Forth Standard](https://forth-standard.org/)


### Assembly Language Notes ###

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

