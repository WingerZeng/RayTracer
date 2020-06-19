#pragma once
#include <memory>
#include "definitions.h"
#include "types.h"
#include "RTObject.h"
class Drawable;
class Sphere;
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
	virtual Vec3 getPosition(HitRecord* rec=nullptr,Vec3* normal=nullptr,double* p=nullptr); //得到随机Pisition，返回概率密度p, 返回法线
	virtual std::shared_ptr<Drawable> getGeometry() {
		return nullptr;
	}
	void setMonteCarloMat();
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
	META_Object(FaceLight);
	//void setSamplePosition(double u, double v) override;
	Vec3 getPosition(HitRecord* rec = nullptr, Vec3* normal = nullptr, double* p = nullptr) override;
protected:
	double uoff_;
	double voff_;
	Vec3 right_;
	Vec3 up_;
	Vec3 normal_;
	double width_;
	double height_;
};

class MovableFaceLight : public FaceLight
{
public:
	MovableFaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular);
	MovableFaceLight(const MovableFaceLight& light, rt::CopyOp copyop);
	META_Object(MovableFaceLight)
	Vec3 getPosition(HitRecord* rec = nullptr, Vec3* normal = nullptr, double* p = nullptr) override;
private:
	double radius_;
	Vec3 orginPos_;
};

class SphereLight : public Light
{
public:
	SphereLight(Vec3 pos, double radius, Color emission);
	SphereLight(const SphereLight& light, rt::CopyOp copyop);
	std::shared_ptr<Drawable> getGeometry() override;
	META_Object(SphereLight);
	Vec3 getPosition(HitRecord* rec = nullptr, Vec3* normal = nullptr, double* p = nullptr) override;
private:
	double radius_;
	std::shared_ptr<Sphere> sphere_;
};