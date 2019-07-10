
#ifndef orbit_cli_options_h
#define orbit_cli_options_h

#include <term/arg.h>


/*
    termArgAddOption(&parser, 'v', "debug-vm", "print debug information (like -x -s)");
    termArgAddOption(&parser, 'n', "syntax-only", "stop compilation before generating code");
    termArgAddOption(&parser, 's', "print-ast", "print syntax tree before running");
    termArgAddOption(&parser, 'x', "print-bytecode", "print compiled bytecode before running");
*/

static const TermParam kOrbitParams[] = {
    {'g', 0, "debug-vm", kTermArgOption, "print debug information (like -x -x)"},
    {'n', 0, "syntax-only", kTermArgOption, "check syntax without generating bytecode"},
    {'s', 0, "print-ast", kTermArgOption, "print the syntax tree when compiling"},
    {'x', 0, "print-bytecode", kTermArgOption, "print compiled bytecode before running"}
};

static const int kOrbitParamCount = 4;

#endif
