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

typedef enum _VMResult  VMResult;
typedef struct _OrbitVM OrbitVM;

enum _VMResult {
    ORBIT_SUCCESS,
    ORBIT_RUNTIME_ERROR
};

OrbitVM* orbit_vmNew();

void orbit_vmDealloc(OrbitVM* vm);

#endif /* orbit_h */
