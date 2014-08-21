/**
 * Header file providing utility for alignment
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef UTIL_ENDIAN_H
#define UTIL_ENDIAN_H

static inline uint32_t endian32(uint32_t val){
    return ((val&0xFF)<<24)|(((val>>8)&0xFF)<<16)|(((val>>16)&0xFF)<<8)|(val>>24);
	// Open the optimization! This function can be changed to bswap
}

#endif
