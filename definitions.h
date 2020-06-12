#pragma once
#include <functional>
class Vec4;
class Vec3;
const double INFINITE = 1e10;
const double ZERO = 1e-8;
const double PI = 3.1415926;

typedef double(*ImpicitFunc)(Vec3);
using StdImpicitFunc = std::function<double(Vec3)>;

typedef Vec4 Color;