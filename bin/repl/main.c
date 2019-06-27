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
#include <orbit/rt2/vm.h>
#include <orbit/rt2/debug.h>
#include <orbit/rt2/opcodes.h>

int main(int argc, const char** argv) {
    OrbitChunk chunk;
    orbit_chunkInit(&chunk);
    orbit_chunkWrite(&chunk, OP_CONST, 123);
    orbit_chunkWrite(&chunk, 0, 123);
    orbit_chunkWrite(&chunk, OP_PRINT, 123);
    orbit_chunkWrite(&chunk, OP_RETURN, 123);
    orbit_arrayAppend(&chunk.constants, 80085);
    
    OrbitVM vm;
    orbit_vmInit(&vm);
    orbit_run(&vm, &chunk);
    orbit_vmDeinit(&vm);
    
    orbit_chunkDeinit(&chunk);
}