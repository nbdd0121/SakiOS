


#include "c/stdint.h"
#include "c/assert.h"

#include "unicode/category.h"

#include "type.inc"

uint8_t unicode_getType(uint32_t ch) {
    switch (ch >> 16) {
        case 0: case 1: case 2: {
            char *pageMap = plane0to2_type_map[ch >> 8];
            if ((size_t)pageMap <= 0xFF) {
                return (uint8_t)(size_t)pageMap;
            }
            return pageMap[ch & 0xFF];
        }
        case 14: {
            char *pageMap = planeE_type_map[(ch >> 8) & 0xFF];
            if ((size_t)pageMap <= 0xFF) {
                return (uint8_t)(size_t)pageMap;
            }
            return pageMap[ch & 0xFF];
        }
        case 15: case 16: // Private Use Planes
            /* Unassigned if surrogate pair */
            return (ch & 0xFFFE) == 0xFFFE ? UNASSIGNED : PRIVATE_USE;
        default:
            return UNASSIGNED;
    }
}

