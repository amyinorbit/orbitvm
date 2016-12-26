# Orbit - Small Scripting Language

A VM-based scripting language, mostly procedural with some swift and a bit of 
objects sprinkled upon.

## Building

At the moment, you can build both static and dynamic libraries for the Orbit
VM and type system:

````bash
$ make liborbit.a
$ make liborbit.dylib
````

You can also build the unit tests (based on [Cutest][1]):

````bash
$ make tests
````

Building has only been tested on macOS (Apple LLVM/clang) so far, but should
work as-is on most Linux/UNIX-based systems and GCC. The only dependancy is the
C standard library.

## Todo list

 * [x] linear-probed hashmap-based dispatch table
 * [ ] everything else


   [1]: https://github.com/mity/cutest