#pragma once

#include <stdint.h>

/* Useful macros */
#define BYTES_TO_BITS(x) ((x) * 8)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define GET_HIGH_WORD(x) (((x) >> 16) & 0xFFFF)
#define GET_LOW_WORD(x) ((x) & 0xFFFF)

#define MAKE_BYTE(hi, lo) (((hi) << 4) | (lo))
#define MAKE_WORD(hi, lo) (((hi) << 8) | (lo))

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/* Useful functions */
inline static int32_t clamp(int32_t x, int32_t min, int32_t max)
{
	return (x < min) ? min : (x > max) ? max : x;
}

inline static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline static uint8_t dec2bcd(uint8_t dec)
{
	return (((dec / 10) << 4) | (dec % 10));
}

inline static uint8_t bcd2dec(uint8_t bcd)
{
	return (((bcd >> 4) * 10) + (bcd & 0x0F));
}
