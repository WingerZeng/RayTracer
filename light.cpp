#include "light.h"
Light::~Light()
{
}

Light::Light(Vec3 pos, Color ambient, Color diffuse, Color specular)
	:pos_(pos),a_(ambient),d_(diffuse),s_(specular)
{
}

Light::Light(const Light & light, rt::CopyOp copyop)
	:RTObject(light,copyop)
{
	pos_ = light.pos_;
	a_ = light.a_;
	d_ = light.d_;
	s_ = light.s_;
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

FaceLight::FaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular)
	:Light(pos,ambient,diffuse,specular),right_(up^normal),up_(up),width_(width),height_(height)
{
	setRandomEnabled(true);
	uoff_ = 0;
	voff_ = 0;
}

FaceLight::FaceLight(const FaceLight & light, rt::CopyOp copyop)
	:Light(light,copyop)
{
	uoff_	=light.uoff_;
	voff_ = light.voff_;
	right_ = light.right_;
	up_ = light.up_;
	width_ = light.width_;
	height_ = light.height_;
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

MovableFaceLight::MovableFaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular)
	:FaceLight(pos,normal,up,width,height,ambient,diffuse,specular)
{
	radius_ = sqrt(pos.x_ * pos.x_ + pos.z_*pos.z_);
	orginPos_ = pos;
}

MovableFaceLight::MovableFaceLight(const MovableFaceLight & light, rt::CopyOp copyop)
	:FaceLight(light,copyop)
{
	radius_ = light.radius_;
	orginPos_ = light.orginPos_;
}

Vec3 MovableFaceLight::getPosition()
{ 
	//ÈÆzÖáÐý×ª
	pos_.x_ = sin(getTime()*2*3.1415926)*radius_;
	pos_.z_ = cos(getTime()*2*3.1415926)*radius_;
	return FaceLight::getPosition();
}
