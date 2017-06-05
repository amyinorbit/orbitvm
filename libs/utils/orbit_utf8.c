//
//  orbit_utf8.c
//  OrbitVM
//
//  Created by Amy Parent on 2017-02-06.
//  Copyright © 2017 Amy Parent. All rights reserved.
//
#include <ctype.h>
#include "orbit_utf8.h"
#include "orbit_utils.h"

#define IN_RANGE(val, lower, upper) ((val) >= (lower) && (val) <= (upper))

codepoint_t utf8_getCodepoint(const char* data, uint64_t length) {
    uint8_t remaining = 0;
    codepoint_t point = 0;
    
    if((*data & 0x80) == 0x00) {
        return *data;
    }
    
    if((*data & 0xe0) == 0xc0) {
        point = *data & 0x1f;
        remaining = 1;
    }
    else if((*data & 0xf0) == 0xe0) {
        point = *data & 0x0f;
        remaining = 2;
    }
    else if((*data & 0xf8) == 0xf0) {
        point = *data & 0x07;
        remaining = 3;
    }
    else { return -1; }
    
    if(remaining > length + 1) { return -1; }
    
    while(remaining > 0) {
        data += 1;
        remaining -= 1;
        if((*data & 0xc0) != 0x80) { return -1; }
        point = (point << 6) | (*data & 0x3f);
    }
    return point;
}


// 0x00000000 - 0x0000007F:
//        0xxxxxxx
//
// 0x00000080 - 0x000007FF:
//    110xxxxx 10xxxxxx
//
// 0x00000800 - 0x0000FFFF:
//    1110xxxx 10xxxxxx 10xxxxxx
//
// 0x00010000 - 0x001FFFFF:
//    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//
// 0x00200000 - 0x03FFFFFF:
//    111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//
// 0x04000000 - 0x7FFFFFFF:
//    1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
int8_t utf8_writeCodepoint(codepoint_t point, char* data, uint64_t length) {
    
    int8_t size =  utf8_codepointSize(point);
    if(size < 0) { return -1; }
    if(size > length) { return -1; }
    
    switch(size) {
    case 1:
        data[0] = point & 0x0000007f;
        return 1;
        break;
        
    case 2:
        data[0] = 0xc0 | ((point & 0x000007c0) >> 6);
        data[1] = 0x80 | (point & 0x0000003f);
        return 2;
        break;
        
    case 3:
        data[0] = 0xe0 | ((point & 0x0000f000) >> 12);
        data[1] = 0x80 | ((point & 0x00000fc0) >> 6);
        data[2] = 0x80 | (point & 0x0000003f);
        return 3;
        break;
        
    case 4:
        data[0] = 0xf0 | ((point & 0x001c0000) >> 18);
        data[1] = 0x80 | ((point & 0x0003f000) >> 12);
        data[2] = 0x80 | ((point & 0x00000fc0) >> 6);
        data[3] = 0x80 | (point & 0x0000003f);
        return 4;
        break;
        
    default:
        break;
    }
    return -1;
}

int8_t utf8_codepointSize(codepoint_t point) {
    if (point < 0) { return -1; }
    if (point <= 0x7f) { return 1; }
    if (point <= 0x7ff) { return 2; }
    if (point <= 0xffff) { return 3; }
    if (point <= 0x10ffff) { return 4; }
    return -1;
}

bool utf8_isPrivate(codepoint_t point) {
    return IN_RANGE(point, 0xe000, 0x0f8ff)
        || IN_RANGE(point, 0xf0000, 0xfffff)
        || IN_RANGE(point, 0x100000, 0x10FFFF);
}

bool utf8_isCombining(codepoint_t point) {
    return IN_RANGE(point, 0x0300, 0x036f);
}

bool utf8_isBMP(codepoint_t point) {
    return IN_RANGE(point, 0x000000, 0x100000) && !utf8_isPrivate(point);
}

bool utf8_isWhitespace(codepoint_t point) {
    return point == 0x0000
        || point == 0x0020
        || point == 0x000d
        || point == 0x0009
        || point == 0x000b
        || point == 0x000c;         
}

bool utf8_isIdentifierHead(codepoint_t point) {
    if(point < 0) { return false; }
    return (point <= 0x7f && (point == '_' || isalpha(point & 0x7f)))
        || (point == 0x00a8)
        || (point == 0x00aa)
        || (point == 0x00ad)
        || (point == 0x00af)
        || (point == 0x2054)
        || IN_RANGE(point, 0x00b2, 0x00b5)
        || IN_RANGE(point, 0x00b7, 0x00ba)
        || IN_RANGE(point, 0x00bc, 0x00be)
        || IN_RANGE(point, 0x00c0, 0x00d6)
        || IN_RANGE(point, 0x00d8, 0x00f6)
        || IN_RANGE(point, 0x00f8, 0x00ff)
        || IN_RANGE(point, 0x0100, 0x02ff)
        || IN_RANGE(point, 0x0370, 0x167f)
        || IN_RANGE(point, 0x1681, 0x180d)
        || IN_RANGE(point, 0x180f, 0x1dbf)
        || IN_RANGE(point, 0x1e00, 0x1fff)
        || IN_RANGE(point, 0x200b, 0x200d)
        || IN_RANGE(point, 0x202a, 0x202e)
        || IN_RANGE(point, 0x203f, 0x2040)
        || IN_RANGE(point, 0x2060, 0x206f)
        || IN_RANGE(point, 0x2070, 0x20cf)
        || IN_RANGE(point, 0x2100, 0x218f)
        || IN_RANGE(point, 0x2460, 0x24ff)
        || IN_RANGE(point, 0x2776, 0x2793)
        || IN_RANGE(point, 0x2c00, 0x2dff)
        || IN_RANGE(point, 0x2e80, 0x2fff)
        || IN_RANGE(point, 0x3004, 0x3007)
        || IN_RANGE(point, 0x3021, 0x302f)
        || IN_RANGE(point, 0x3031, 0x303f)
        || IN_RANGE(point, 0x3040, 0xd7ff)
        || IN_RANGE(point, 0xf900, 0xfd3d)
        || IN_RANGE(point, 0xfd40, 0xfdcf)
        || IN_RANGE(point, 0xfdf0, 0xfe1f)
        || IN_RANGE(point, 0xfe30, 0xfe44)
        || IN_RANGE(point, 0xfe47, 0xfffd)
        || IN_RANGE(point, 0x10000, 0x1fffd)
        || IN_RANGE(point, 0x20000, 0x2fffd)
        || IN_RANGE(point, 0x30000, 0x3fffd)
        || IN_RANGE(point, 0x40000, 0x4fffd)
        || IN_RANGE(point, 0x50000, 0x5fffd)
        || IN_RANGE(point, 0x60000, 0x6fffd)
        || IN_RANGE(point, 0x70000, 0x7fffd)
        || IN_RANGE(point, 0x80000, 0x8fffd)
        || IN_RANGE(point, 0x90000, 0x9fffd)
        || IN_RANGE(point, 0xa0000, 0xafffd)
        || IN_RANGE(point, 0xb0000, 0xbfffd)
        || IN_RANGE(point, 0xc0000, 0xcfffd)
        || IN_RANGE(point, 0xd0000, 0xdfffd)
        || IN_RANGE(point, 0xe0000, 0xefffd);
}

bool utf8_isIdentifier(codepoint_t point) {
    return utf8_isIdentifierHead(point)
        || (point <= 0x7f && isdigit(point & 0x7f))
        || IN_RANGE(point, 0x0300, 0x036f)
        || IN_RANGE(point, 0x1dc0, 0x1dff)
        || IN_RANGE(point, 0x20d0, 0x20ff)
        || IN_RANGE(point, 0xfe20, 0xfe2f);
}

bool utf8_isOperator(codepoint_t point) {
    if(point > 0x7f) { return false; }
    return (point & 0x7f) == '+'
        || (point & 0x7f) == '-'
        || (point & 0x7f) == '*'
        || (point & 0x7f) == '/'
        || (point & 0x7f) == '%'
        || (point & 0x7f) == '?'
        || (point & 0x7f) == '<'
        || (point & 0x7f) == '>'
        || (point & 0x7f) == '!'
        || (point & 0x7f) == '|'
        || (point & 0x7f) == '&'
        || (point & 0x7f) == '^'
        || (point & 0x7f) == '~';
}

bool utf_isQuotedItem(codepoint_t point) {
    if(point < 0 && point > 0x7f) { return false; }
    return point != '\\' && point != '"' && point != 0x0a && point != 0x0d;
}
