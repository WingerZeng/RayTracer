#pragma once
#include "definitions.h"
#include "types.h"
class ToneReproductionMethod
{
public:
	virtual ~ToneReproductionMethod() {};
	virtual Color reproduction(Color color)=0; //reproduction color to 0~1
};

class SigmoidMethod: public ToneReproductionMethod
{
public:
	Color reproduction(Color color) override {
		color.x_ = sigmoid(color.x_);
		color.y_ = sigmoid(color.y_);
		color.z_ = sigmoid(color.z_);
		return color;
	};
private:
	inline double sigmoid(double x) {
		return 1.0 / (1 + exp(-(2 * x - 1)))*2.164 - 0.582;
	}
};

class SqrtMethod: public ToneReproductionMethod
{
public:
	Color reproduction(Color color) override {
		color.x_ = color.x_ > 1 ? pow(color.x_, 1.0 / 2): color.x_;
		color.y_ = color.y_ > 1 ? pow(color.y_, 1.0 / 2) : color.y_;
		color.z_ = color.z_ > 1 ? pow(color.z_, 1.0 / 2) : color.z_;
		return color;
	};
};