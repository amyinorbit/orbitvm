//
//  orbit_stdlib.h
//  OrbitVM
//
//  Created by Cesar Parent on 2017-02-10.
//  Copyright © 2017 cesarparent. All rights reserved.
//
#ifndef orbit_stdlib_h
#define orbit_stdlib_h

#include "orbit_platforms.h"

typedef struct _OrbitVM OrbitVM;

void orbit_registerStandardLib(OrbitVM* vm);

#endif /* orbit_stdlib_h */
