# OrbitVM Name Mangling Reference

Because Orbit is planned to support function overloads, name mangling is necessary (in order for symbolic invocation to only match the correct implementation of the called function). The chosen scheme is still very fluid and might change depending on requirements (and before the module file format and compiler specifications are frozen).

## Keywords

| Keyword   | Description                                       |
|:----------|:--------------------------------------------------|
| `N`       | Denotes a native Orbit type following             |
| `b`       | Orbit native `Bool`                               |
| `*`       | Orbit native `Any`                                |
| `d`       | Orbit native `Number` (double)                    |
| `i`       | Orbit native `Int` (unused, reserved)             |
| `s`       | Orbit native `String`                             |
| `a`       | Orbit array type                                  |
| `m`       | Orbit dictionary type                             |
| `U`       | Denotes a user type following                     |
| `v`       | Void type                                         |
| `f`       | Function type                                     |
| `p`       | Begins a list of function parameters              |
| `t`       | Begins a list of template parameters              |
| `e`       | Ends a type list                                  |

## Identifiers

Orbit does not support member functions (methods, static functions). It does however have the notion of modules (each orbit source files defines a module). Function names should not be required to be unique across modules, but need to be unique in the runtime dispatch table.

Identifiers are preceded by their length. Function and variable names are mangled so that their modules are added before them. A `Player` type defined in `GamePlay.orbit` would thus become `8GamePlay6Player`. 

TODO: define encoding scheme for non-ascii characters in identifiers (probably `\x1234` unicode codepoints?)

## Grammar

````
mangled-func    ::= full-identifier func-type

type-list       ::= type-name ('_' type-name)*
type-name       ::= 'U' full-identifier
                  | 'N' orbit-type-id
                  | func-type
                  | array-type
                  | map-type
                  | 'v'
func-params     ::= 'p' type-list 'e'
template-params ::= 't' type-list 'e'

func-type       ::= 'f' (func-params)? '_' type-name
array-type      ::= 'a' template-params
map-type        ::= 'm' template-params

orbit-type-id   ::= 'b' | '*' | 'd' | 'i' | 's'

full-identifier ::= (identifier)? identifier
identifier      ::= [0-9]+ identifier-head (identifier-char)*

````

