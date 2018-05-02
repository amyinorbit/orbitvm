# OrbitVM Name Mangling Reference

Because Orbit is planned to support function overloads, name mangling is necessary (in order for symbolic invocation to only match the correct implementation of the called function). The chosen scheme is still very fluid and might change depending on requirements (and before the module file format and compiler specifications are frozen).

## Keywords

| Keyword   | Description                                       |
|:----------|:--------------------------------------------------|
| `O`       | Denotes a native Orbit type following             |
| `Ob`      | Orbit native `Bool`                               |
| `Od`      | Orbit native `Number` (double)                    |
| `Oi`      | Orbit native `Int` (unused, reserved)             |
| `Os`      | Orbit native `String`                             |
| `U`       | Denotes a user type following                     |
| `f`       | Begins a list of function parameters              |
| `t`       | Begins a list of template parameters              |
| `e`       | Ends a type list                                  |


## Grammar

````

````

