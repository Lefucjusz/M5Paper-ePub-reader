#pragma once

#include <stdint.h>

/* Useful macros */
#define BYTES_TO_BITS(x) ((x) * 8)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define GET_HIGH_WORD(x) (((x) >> 16) & 0xFFFF)
#define GET_LOW_WORD(x) ((x) & 0xFFFF)

#define MAKE_BYTE(hi, lo) (((hi) << 4) | (lo))
#define MAKE_WORD(hi, lo) (((hi) << 8) | (lo))

/* Useful functions */
inline static int32_t clamp(int32_t x, int32_t min, int32_t max)
{
	return (x < min) ? min : (x > max) ? max : x;
}

inline static int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline static int32_t min(int32_t x, int32_t y)
{
	return (x < y) ? x : y;
}

inline static int32_t max(int32_t x, int32_t y)
{
	return (x > y) ? x : y;
}
