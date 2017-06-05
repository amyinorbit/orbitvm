//
//  orbit/parser/type.h
//  Orbit - Parser
//
//  Created by Amy Parent on 2017-05-26.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_type_h
#define orbit_type_h

#include <stdio.h>
#include <stdbool.h>

typedef struct _OCType OCType;
typedef enum _OCTypeKind OCTypeKind;

enum _OCTypeKind{
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_MAP,
    TYPE_FUNCTION,
    TYPE_USER,
};

struct _OCType {
    OCTypeKind  kind;
    bool        isConst;
    OCType*     returnType;
    OCType*     sibling;
    OCType*     children;
    const char* name;
};

#define MAKE_TYPE(kind) ((OCType){(kind), false, NULL, NULL, NULL, NULL})
#define MAKE_CONST(kind) ((OCType){(kind), true, NULL, NULL, NULL, NULL})

extern OCType* orbit_Void;
extern OCType* orbit_Bool;
extern OCType* orbit_Number;
extern OCType* orbit_String;

void orbit_typeCheckerInit();
void orbit_typeCheckerDeinit();
void orbit_typePrint(FILE* out, const OCType* type);
bool orbit_typeEqual(const OCType* typeA, const OCType* typeB);
OCType* orbit_typeInstall(OCType* type);


#endif /* orbit_type_h */
