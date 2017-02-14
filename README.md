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

You can also build the unit tests (based on [Unity][1]):

````bash
$ make tests
````

Building has only been tested on macOS (Apple LLVM/clang) so far, but should
work as-is on most Linux/UNIX-based systems and GCC. The only dependancy is the
C standard library.

## Roadmap

At the moment, OrbitVM is capable of running most of its bytecode instructions
and call foreign functions (defined from the C API). The garbage collector
(simple mark-and-sweep) is operational. A (private so far) simple assembler
can generate Orbit Module Files, and OrbitVM can load them and run their 
bytecode.

The immediate next steps are:

 * [ ] improve module loading and registration in the VM
 * [ ] add dependency import system
 * [ ] improve UTF-8 support and string library
 * [ ] implement a basic standard library
 * [ ] improve runtime error printing, and provide some error handling

Once this is done, the next main step will be the compiler:

 * [ ] define complete Orbit grammar
 * [-] describe Orbit type system
 * [ ] implement Recursive-descent parser
 * [ ] provide AST walker API:
     * [ ] implement OMF backend (source to orbit module file)
     * [ ] implement direct codegen backend (source to VM, REPL)

   [1]: http://www.throwtheswitch.org/unity/