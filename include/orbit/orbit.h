//
//  orbit.h
//  OrbitVM
//
//  Created by Cesar Parent on 09/12/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//

#ifndef orbit_h
#define orbit_h

#include <stdint.h>
#include <stddef.h>

typedef struct _OrbitVM     OrbitVM;
typedef struct _OrbitTask   OrbitTask;
typedef struct _OrbitModule OrbitModule;

typedef enum _OrbitResult OrbitResult;

enum _OrbitResult {
    ORBIT_SUCCESS,
    ORBIT_RUNTIME_ERROR
};


extern OrbitVM* orbit_createVM();

extern OrbitModule* orbit_loadModule(uint8_t* bytes, size_t size);

extern OrbitResult orbit_runByteCode(OrbitVM* vm, OrbitModule* task, const char* main);

extern void orbit_destroyVM(OrbitVM* vm);


#endif /* orbit_h */
