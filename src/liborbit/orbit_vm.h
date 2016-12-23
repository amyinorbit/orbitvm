//
//  orbit_vm.h
//  OrbitVM
//
//  Created by Cesar Parent on 08/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//

#ifndef orbit_vm_h
#define orbit_vm_h

#include <stdio.h>
#include <orbit/orbit.h>

#include "orbit_value.h"

#define ORBIT_STACK_SIZE 2048

typedef struct _OrbitTask OrbitTask;
typedef struct _OrbitFrame OrbitFrame;

/**
 *
 *
 */
struct _OrbitVM {
    OrbitValue*     sp;
    OrbitValue      stack[ORBIT_STACK_SIZE];
    
    OrbitInstance*  collectorHead;
    size_t          allocations;
};

/**
 *
 *
 *
 */
struct _OrbitFrame {
    OrbitTask*      task;
    OrbitFunction*  function;
    OrbitValue*     locals;
    OrbitValue*     bp;
};

/**
 *
 *
 *
 */
struct _OrbitModule {
    OrbitVM*        vm;
    uint16_t        globalCount;
    OrbitValue*     globals;
};

/**
 *
 *
 *
 */
struct _OrbitTask {
    OrbitVM*        vm;
    size_t          frameCount;
    OrbitFrame      callStack[ORBIT_STACK_SIZE];
};



#endif /* orbit_vm_h */
