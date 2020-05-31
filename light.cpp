#include "light.h"
Light::~Light()
{
}

Light::Light(Vec3 pos, Color ambient, Color diffuse, Color specular)
	:pos_(pos),a_(ambient),d_(diffuse),s_(specular)
{
}

Color Light::getDiffuse()
{
	return d_;
}

Color Light::getAmbient()
{
	return a_;
}

Color Light::getSpecular()
{
	return s_;
}

Vec3 Light::getPosition()
{
	return pos_;
}

void Light::setSamplePosition(double u, double v)
{
}

FaceLight::FaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular)
	:Light(pos,ambient,diffuse,specular),right_(up^normal),up_(up),width_(width),height_(height)
{
	setRandomEnabled(true);
	uoff_ = 0;
	voff_ = 0;
}

//void FaceLight::setSamplePosition(double u, double v)
//{
//	uoff_ = u-0.5;
//	voff_ = v-0.5;
//}

Vec3 FaceLight::getPosition()
{
	std::pair<double, double> uv;
	getRandomPair(&uv);
	Vec3 pos = Light::getPosition();
	pos += right_ * (uv.first-0.5) * width_ + up_ * (uv.second-0.5) * height_;
	return pos;
}
