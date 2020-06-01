#pragma once
#include "scene.h"

void getRayColor(Scene* scene, std::vector<Ray> ray, double t0, double t1, int jumptime, std::vector<Color>& color);