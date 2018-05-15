# Orbit - Small Scripting Language

A VM-based scripting language, mostly procedural with some swift and a bit of 
objects sprinkled upon.

## Building

Building Orbit's different modules requires a C compiler (GCC or Clang), CMake
and some build system (GNU Make, Ninja, Xcode, VS...)

````bash
$ mkdir build
$ cd build
$ cmake ..
$ cd ..

# build the project
$ cmake --build build --target all

# or install the binaries and libraries
$ cmake --build build --target install
````

Building has only been tested on macOS (Apple LLVM/clang) so far, but should
work as-is on most Linux/UNIX-based systems and GCC. The only dependancy is the
C standard library.

## Contributing

Orbit is a very small project, and I don't expect it to pick up a lot of
contributors. If you feel like working on it, make sure you have read and
respect the [code of conduct][2], and fork away. Given the size, I'll review
the pull requests myself, but in the future a discussion system might be more
appropriate.

If you contribute, I strongly encourage you to use the pre-commit hook provided
in `/tools/git-hooks`:

````bash
$ cd .git/hooks
$ ln -s ../../tools/git-hooks/pre-commit.sh pre-commit
$ chmod +x pre-commit
````

The hook will ensure that all the automated tests pass before committing. If you
are working locally and want to commit some temporary work, you can use bypass it
with `git commit --no-verify`, but it is heavily recommended that you don't push
any changes that do not pass the tests.

   [1]: http://www.throwtheswitch.org/unity/
   [2]: CODE_OF_CONDUCT.md
