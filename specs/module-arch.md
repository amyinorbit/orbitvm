# Module Architecture

The goal of this document is to establish a proper module architecture for the Orbit language and compiler.

## Requirements:

The module system should be able to:

1. encapsulate functions and variables. Globals should be tied to a module, as should functions.
2. make imports easier. Importing another file just means compiling it, then making its Module available to the VM.
3. Allow the REPL to have persistence between calls.
4. top-level (script) code should be an anonymous function in the module

## Info required by compiler stages

 * parser: <nil>
 * sema:
    - are we compiling a function into an existing module?
    - context for the previous module if there.
        - context should contain a symbols table
        - context should contain function declarations
 * codegen:
    - are we generating code that ties into an existing module?
    - module we are compiling into (emit globals/function calls)

## Potential solutions:

### 1. Light changes, pass compiled module around:
 - OrbitModule contains a function and variable table
 - A module pointer is passed in OrbitASTContext [nullable]

#### potential issues
 - More compiler stages tied to runtime module than ideal (requires a VM/garbage collector to be passed around.
 - More changes required when we move to split-design (compiler -> il/bc -> VM)

### 2. Create a compiler-side representation of a module.
 - This could be a light wrapper around OrbitAST -- hold pointers to nodes, collection for globals and functions
 - Need to mark functions that have already been codegen-d

#### potential issues
 - More data structures that need to be kept in sync
