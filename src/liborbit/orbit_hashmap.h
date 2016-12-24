//
//  orbit_hashmap.h
//  OrbitVM
//
//  Created by Cesar Parent on 22/11/2016.
//  Copyright © 2016 cesarparent. All rights reserved.
//
#ifndef orbit_dispatch_h
#define orbit_dispatch_h

#include <stdint.h>
#include <stdbool.h>
#include "orbit_platforms.h"


// TODO: keep a reference to the signature's constant, so that we can just
// compare by pointer
// TODO:(later): hey dummy, the selector* pointer already points to the
//      constant's bytes

typedef struct _HashBucket {
    const char* key;
    bool        used;
    void*       data;
} HashBucket;

typedef struct _HashMap {
    uint32_t    mask;
    uint32_t    capacity;
    uint32_t    size;
    HashBucket  data[ORBIT_FLEXIBLE_ARRAY_MEMB];
} HashMap;


#ifndef ORBIT_MAP_DEFAULT_SIZE
#define ORBIT_MAP_DEFAULT_SIZE 32
#endif

extern HashMap* orbit_hashmapNew(void);

extern HashMap* orbit_hashmapInsert(HashMap* map, const char* key, void* data);

extern void* orbit_hashmapGet(HashMap* map, const char* key);

extern void orbit_hashmapDealloc(HashMap* map);

#endif /* orbit_dispatch_h */
