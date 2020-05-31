#pragma once
#include <algorithm>
#include "types.h"

namespace algorithm {
	inline void randFromDisk(double* x, double* y) {
		do {
			*x = (rand() * 1.0 / RAND_MAX - 0.5) * 2;
			*y = (rand() * 1.0 / RAND_MAX - 0.5) * 2;
		} while (*x * *x + *y * *y >= 1.0);
	}
}