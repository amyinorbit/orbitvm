
#ifndef orbit_cli_options_h
#define orbit_cli_options_h

#include <term/arg.h>

static const TermParam kOrbitParams[] = {
    {'g', 0, "debug-vm", kTermArgOption, "print debug information\nsame as -s -x"},
    {'n', 0, "syntax-only", kTermArgOption, "check syntax without generating bytecode"},
    {'s', 0, "print-ast", kTermArgOption, "print the syntax tree when compiling"},
    {'x', 0, "print-bytecode", kTermArgOption, "print compiled bytecode before running"}
};
static const int kOrbitParamCount = 4;

static const char* kOrbitUses[] = {
    "[option]... source_file",
    "[option]..."
};
static const int kOrbitUseCount = 2;

static const char* kOrbitWebsite = "https://github.com/amyinorbit/orbitvm";
static const char* kOrbitEmail = "amy@amyparent.com";

#endif
