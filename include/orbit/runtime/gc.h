//
//  orbit_gc.h
//  OrbitVM
//
//  Created by Amy Parent on 2017-01-12.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_gc_h
#define orbit_gc_h

#include <orbit/orbit.h>
#include <orbit/runtime/value.h>
#include <orbit/runtime/vm.h>

void orbit_gcRun(OrbitVM* vm);

void orbit_gcMarkObject(OrbitVM* vm, GCObject* obj);

void orbit_gcMark(OrbitVM* vm, GCValue value);

#endif /* orbit_gc_h */
