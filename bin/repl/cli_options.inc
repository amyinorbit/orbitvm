
#ifndef orbit_cli_options_h
#define orbit_cli_options_h

#include <term/arg.h>

#define COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

static const TermParam kOrbitParams[] = {
    {'g', 0, "debug-vm", kTermArgOption, "print debug information\nsame as --print-ast --print-bytecode"},
    {'n', 0, "syntax-only", kTermArgOption, "check syntax without generating bytecode"},
    {'t', 0, "print-ast", kTermArgOption, "print the syntax tree when compiling"},
    {'x', 0, "print-bytecode", kTermArgOption, "print compiled bytecode before running"},
    {'o', 0, "output", kTermArgValue, "specify a file to write outputs to\nonly has an effect with --emit-* options"},
    {'S', 0, "emit-assembly", kTermArgOption, "write disassembled bytecode to the output file"},
    {'T', 0, "emit-ast", kTermArgOption, "write the syntax tree to the output file"},
};
static const int kOrbitParamCount = COUNTOF(kOrbitParams);

static const char* kOrbitUses[] = {
    "[option]... source_file",
    "[option]..."
};
static const int kOrbitUseCount = COUNTOF(kOrbitUses);

static const char* kOrbitWebsite = "https://github.com/amyinorbit/orbitvm";
static const char* kOrbitEmail = "amy@amyparent.com";

#endif
