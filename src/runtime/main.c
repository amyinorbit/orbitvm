#include <stdio.h>
#include <string.h>
#include <time.h>
#include <orbit/orbit.h>

int main(int argc, const char** argv) {
    
    if(argc < 2) {
        fprintf(stderr, "error: no input module file.\n");
        return -1;
    }
    OrbitVM* vm = orbit_vmNew();
    
    clock_t start = clock();
    bool success = orbit_vmInvoke(vm, argv[1], "main");
    clock_t end = clock();
    
    double elapsed_time = (end - start)/(double)CLOCKS_PER_SEC;
    printf("----- orbit done -----\n");
    printf("cpu time: %.2f\n", elapsed_time);
    
    if(!success) {
        fprintf(stderr, "error: interpreter error\n");
    }
    
    orbit_vmDealloc(vm);
}