#include <stdio.h>
#include <string.h>
#include <time.h>
#include <orbit/mangling/mangle.h>
#include <orbit/utils/string.h>
#include <orbit/stdlib/stdlib.h>
#include <orbit/orbit.h>

static void demangle(const char* name) {
    OCStringID id = orbit_demangle(name, strlen(name));
    if(id == orbit_invalidStringID) {
        fprintf(stderr, "error: unable to demangle `%s`\n", name);
    } else {
        OCString* str = orbit_stringPoolGet(id);
        printf("%s ---> %.*s\n", name, (int)str->length, str->data);
    }
}

static bool console_getLine(char *buffer, size_t size) {
    int ch, extra;
    if (fgets(buffer, size, stdin) == NULL) { return false; }

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buffer[strlen(buffer)-1] != '\n') {
        extra = 0;
        while(((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? false : true;
    }
    // Otherwise remove newline and give string back to caller.
    buffer[strlen(buffer)-1] = '\0';
    return true;
}

static void demangleLoop() {
    char name[512];
    //sscanf(const char *restrict, const char *restrict, ...)
    while(true) {
        if(!console_getLine(name, 512)) { continue; }
        OCString* demangled = orbit_stringPoolGet(orbit_demangle(name, strlen(name)));
        if(demangled) {
            printf("%.*s\n", (int)demangled->length, demangled->data);
        } else {
            fprintf(stderr, "error: unable to demangle `%s`\n", name);
        }
    }
}

int main(int argc, const char** argv) {
    
    if(argc < 2) {
        fprintf(stderr, "error: no input module file.\n");
        return -1;
    }
    orbit_stringPoolInit(512);
    
    const char* command = argv[1];
    
    if(strcmp(command, "demangle") == 0) {
        if(argc >= 3) {
            demangle(argv[2]);
        } else {
            demangleLoop();
        }
    }
    else {
        OrbitVM* vm = orbit_vmNew();
        orbit_registerStandardLib(vm);
        if(!orbit_vmInvoke(vm, command, "main")) {
            fprintf(stderr, "error: interpreter error\n");
        }
        orbit_vmDealloc(vm);
    }
    orbit_stringPoolDeinit();
}