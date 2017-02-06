# OrbitVM - Language Specification

## Grammar

    // Declarations
    <decl>      ::= "var" <identifier> (":" <type>)? ("=" <expression>)
    <assign>    ::= <identifier> "=" <expression>
    
    // Expressions. THE LOL
    
    <expression>::= 
    
    // Literals
    
    <string>    ::= "<utf-8-characters>"

    // Types
    
    <struct>    ::= "struct" <identifier> "{" <declist> "}"
    <declist>   ::= <decl> | <decl> <declist> | <>

    <type>      ::= <typename>
                  | "Array<" <typename> ">"
                  | "Map<" <typename> "," <typename> ">"
                  
                  
    <typename>  ::= "Number" | "Float" | "Boolean" | <identifier>
    <identifier>::= (UTF-8 id)
  

## Type System

While OrbitVM provides support for dynamic typing, the default implementation
of the Orbit compiler enforces strong typing.

### Default Types

type        | semantics | comments
:-----------|:----------|-----------------------------------------------
Nil         | value     |
Bool        | value     | Boolean logic value (True or False)
Number      | value     | mutable double float
String      | reference | immutable UTF-8 string
Array       | reference | contiguous mutable collection type
Dict        | reference | associative array type

`Number` and `String` are primitive types, and can be used as keys into a
`Dict`.

### User Types

Orbit provides a way for users to defines their own types, `struct`s. Structs
group related data together as their fields. `struct` as reference semantics,
and all user-defined types are garbage-collected. A `struct` definition must
contain at least one field.

User-defined types are instanced by a call to `init(type: Class) : Class`:


    struct UserType {
        var fieldName : Type
        [...]
    }
    
    var instance = init(UserType)
    var instance : UserType = init(UserType)

### Variable Declaration

