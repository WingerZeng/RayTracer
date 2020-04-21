#pragma once
#include "definitions.h"
#include "types.h"

class Light
{
public:
	Light(Vec3 pos,Color ambient,Color diffuse,Color specular);
	virtual ~Light();
	virtual Color getDiffuse();
	virtual Color getAmbient();
	virtual Color getSpecular();
	virtual Vec3 getPosition();
	virtual void setSamplePosition(double u, double v);

private:
	Vec3 pos_;
	Color a_;
	Color d_;
	Color s_;
};

class FaceLight : public Light
{
public:
	FaceLight(Vec3 pos,Vec3 normal,Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular);
	void setSamplePosition(double u, double v) override;
	Vec3 getPosition() override;
private:
	double uoff_;
	double voff_;
	Vec3 right_;
	Vec3 up_;
	double width_;
	double height_;
};
