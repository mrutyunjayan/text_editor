/* date = September 7th 2021 11:34 am */

#ifndef JED_INTRINSICS_H
#define JED_INTRINSICS_H

typedef struct BitscanResult {
	b32 found;
	u32 index;
} BitscanResult;

inline internal BitscanResult
findLeastSignificantSetBit(u32 value) {
	BitscanResult result = {0};
#if COMPILER_CLANG
	// NOTE(Jai): __builtin_ctz returns '0' if value not found
	// or if the value is on index 0. Curently assuming value is
	// found (at index 0) when it returns 0.
	result.index = (u32)__builtin_ctz(value);
	result.found = true;
#else
	for (u32 test = 0; test < 32; test++) {
		if (value & (1 << test)) {
			result.index = test;
			result.found = true;
			break;
		}
	}
#endif
	return result;
}

#endif //JED_INTRINSICS_H
