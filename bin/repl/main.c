//===--------------------------------------------------------------------------------------------===
// main.c - Entry point for the Orbit Runtime 2.0 REPL
// This source is part of Orbit
//
// Created on 2019-06-27 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2016-2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <stdio.h>
#include <orbit/rt2/chunk.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>

int main(int argc, const char** argv) {
    OrbitChunk chunk;
    orbit_chunkInit(&chunk);
    orbit_chunkWrite(&chunk, OP_IADD, 123);
    orbit_chunkWrite(&chunk, OP_RETURN, 123);
    
    orbit_debugChunk(&chunk, "<main>");
    
    orbit_chunkDeinit(&chunk);
}