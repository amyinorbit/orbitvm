# Orbit - Small Scripting Language

A VM-based scripting language, mostly procedural with some swift and a bit of 
objects sprinkled upon.

## Execution Model

Script files are compiled to bytecode files by `orbitc`. When a bytecode file
is loaded, a `VMContext` object is created. A context holds its own global
variables, dispatch table and available user types. Once the context is created,
the program can be started by invoking a function.

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
 * [x] basic garbage collector
 * [x] Garbage-collected object and value system
 * [x] Object file layout design
 * [x] Stack allocation and growth
 * [ ] Basic Standard Library
 * [ ] basic assembly language and assembler
 * [ ] everything else


   [1]: https://github.com/mity/cutest