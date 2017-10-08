//
//  orbit/type/builders.h
//  Orbit - AST
//
//  Created by Amy Parent on 2017-10-07.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#ifndef orbit_type_builders_h
#define orbit_type_builders_h

#include <stdio.h>
#include <stdbool.h>
#include <orbit/type/type.h>

Type* type_makeArray(Type* elementType);
Type* type_makeMap(Type* keyType, Type* elementType);
Type* type_makeFunction(Type* returnType, Type* paramTypes);
Type* type_makeUserType(OCToken* symbol);

#endif /* orbit_type_builders_h */
