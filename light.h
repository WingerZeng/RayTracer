#pragma once
#include "definitions.h"
#include "types.h"
#include "RTObject.h"

class Light: public RTObject
{
public:
	Light(Vec3 pos,Color ambient,Color diffuse,Color specular);
	Light(const Light& light, rt::CopyOp copyop);
	virtual ~Light();

	META_Object(Light)

	virtual Color getDiffuse();
	virtual Color getAmbient();
	virtual Color getSpecular();
	virtual Vec3 getPosition();

protected:
	Vec3 pos_;
	Color a_;
	Color d_;
	Color s_;
};

class FaceLight : public Light
{
public:
	FaceLight(Vec3 pos,Vec3 normal,Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular);
	FaceLight(const FaceLight& light, rt::CopyOp copyop);
	META_Object(FaceLight)
	//void setSamplePosition(double u, double v) override;
	Vec3 getPosition() override;
private:
	double uoff_;
	double voff_;
	Vec3 right_;
	Vec3 up_;
	double width_;
	double height_;
};

class MovableFaceLight : public FaceLight
{
public:
	MovableFaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular);
	MovableFaceLight(const MovableFaceLight& light, rt::CopyOp copyop);
	META_Object(MovableFaceLight)
	Vec3 getPosition() override;
private:
	double radius_;
	Vec3 orginPos_;
};