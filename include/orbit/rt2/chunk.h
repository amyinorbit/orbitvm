//===--------------------------------------------------------------------------------------------===
// chunk.h - Chunks are fundamental units of bytecode in Orbit
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef orbit_chunk_h
#define orbit_chunk_h
#include <orbit/common.h>

typedef struct sOrbitChunk OrbitChunk;
//typedef struct sOrbitLineData OrbitLineData;
typedef uint32_t OrbitLineData;

struct sOrbitLineData {
    uint32_t line;
    uint32_t count;
};

struct sOrbitChunk {
    size_t count;
    size_t capacity;
    uint8_t* code;
    
    OrbitLineData* lines;
};

void orbit_chunkInit(OrbitChunk* self);
void orbit_chunkDeinit(OrbitChunk* self);
void orbit_chunkWrite(OrbitChunk* self, uint8_t byte, uint32_t line);

#endif /* orbit_chunk_h */