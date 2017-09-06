OrbitVM - Type System Manifesto
===============================

Orbit needs a type system. At the moment, it's a weird bastard between dynamic
and static typing. The compiler (or parser really, since that's the one part
that's pretty much done) allows type annotations on variables, and the compiler
will ultimately provide type inference, and enforce static typing.

On the VM side however, things are less strict for the time being. Each variable
and value is allocated using a GCValue tagged union, much like in dynamic
languages. I'm considering keeping it that way, for the sake of simplicity.

Even so, two things need to be defined before I make any more significant
progress on the compiler. Delaying those decisions more will only lead to
painful shoe-horning down the road.

 * The compile-time type system. What types exist, what can and can't be done,
   what does the compiler guess and what do users specify. That includes not
   only primitives, but also whether I want generics to be limited to language
   collections (`Array` and `Map`) or have a user-defined generics system
   (ouch).

 * The runtime type system. Especially if Orbit has a `Any` type like it has
   now, users must be able to query the type of an object at runtime. If the
   type system doesn't allow generics, that kind be pretty much just based on
   strings containing a normalised version of the type's name. If users can
   define generic types, it gets much more complex, and probably requires much
   of the compiler's type table system to be included in the VM too.

Type System, V1
---------------

There's a few things I'm certain I want in orbit's user-exposed type system:

 a. Static: I know some people like dynamic systems, but I've found they tend
    to lead to errors more than anything.
   
 b. Strong typing: While it can seem constraining at first, I very much love
    Swift's type system, and that'll probably be the model for strictness in
    orbit. That has some impact on the runtime type system too.
 
 c. At least some level of generic support. The bare minimum is supporting
    generic primitive collection types (`Array` and `Dict`). Ideally, I'd like
    to have at least some support for user-defined generic functions, but it
    means big changes for the way user-defined types are stored at runtime.

 d. Type-inferred: This is 100% on the compiler side. I feel like any language
    designed today should be able to infer as much as possible. One limitation:
    the user should *always* be able to bypass type inference through type
    annotations.

 e. Maybe optional support. Yes, it can add some checking code, but (1) that's
    code that should have been written to check for `nil` anyway, and (2) it makes
    possible nil-ness explicit, and leads to safer programs.

(c) is the big one here. Even generic collection types are sneakier than they
appear at first glance, since they can be passed as parameters, and thus
require some sort of generic function system. The other solutions are (1) force
the users to write a function for each version of `Array<T>` they use (not
awesome) or (2) convert any `Array<T>` parameter to `Array<Any>`, but that
flies in the face of the strong type system (b).

As far as primitives are concerned, so far the list includes the following.
`Number` will very probably be split in `Int` and `Float` at some point.

 * `Void`, nothing. Only allowed as return type, not exposed to the VM,
 * `(Params)->ReturnType`, Generic type for all functions,
 * `String`, UTF-8 backed unicode string,
 * `Number`, any number (stored as double),
 * `Any`, for variables that need to contain anything. Most likely only allowed
   as function or type parameter.
 * `Array<T>`, linear random-access collection type
 * `Map<K,V>`, hash table-backed dictionary type.
 
