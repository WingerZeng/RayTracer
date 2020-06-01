#pragma once
#include <algorithm>
#include <iostream>
#include <memory>
#include "definitions.h"
class Vec3;
std::ostream& operator<<(std::ostream& ios, const Vec3& rhs);
class Vec3
{
public:
	Vec3()
		:x_(0), y_(0), z_(0) {}
	Vec3(double x,double y,double z)
		:x_(x),y_(y),z_(z){}
	double x_;
	double y_;
	double z_;

	Vec3 normalize() {
		//std::cout << *this <<std::endl;
		//std::cout << *this * (1 / length()) << std::endl;
		return *this* (1 / length());
	}

	double length() const{
		return sqrt(x_ * x_ + y_ * y_ + z_ * z_);
	}

	bool isZero() {
		return (abs(x_) < ZERO) && (abs(y_) < ZERO) && (abs(z_) < ZERO);
	}

	Vec3 operator*(const double& rhs) const{
		return Vec3(x_ * rhs, y_ * rhs, z_ * rhs);
	}

	Vec3 reflact(const Vec3& normal) const{
		Vec3 proj = -normal*(*this * normal);
		return (proj * 2 + *this).normalize();
	}

	Vec3 operator-()  const {
		return Vec3(-x_, -y_, -z_);
	}

	Vec3 operator-(const Vec3& rhs)  const {
		return Vec3(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
	}

	double operator*(const Vec3& rhs) const{
		return x_ * rhs.x_ + y_* rhs.y_ + z_ * rhs.z_;
	}


	Vec3 operator+(const Vec3& rhs)  const {
		return Vec3(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
	}

	Vec3 operator^(const Vec3& rhs)  const {
		double a = x_;
		double b = y_;
		double c = z_;
		double d = rhs.x_;
		double e = rhs.y_;
		double f = rhs.z_;
		return Vec3(b * f - e * c, c * d - a * f, a * e - d * b);
	}

	Vec3& operator+=(const Vec3& rhs) {
		x_ += rhs.x_;
		y_ += rhs.y_;
		z_ += rhs.z_;
		return *this;
	}

	Vec3& operator=(const Vec3& rhs) {
		x_ = rhs.x_;
		y_ = rhs.y_;
		z_ = rhs.z_;
		return *this;
	}
};

class Vec4;
std::ostream& operator<<(std::ostream& ios, const Vec4& rhs);
class Vec4
{
public:
	Vec4()
		:x_(0), y_(0), z_(0), w_(0) {}
	Vec4(double x, double y, double z ,double w)
		:x_(x), y_(y), z_(z), w_(w) {}
	double x_;
	double y_;
	double z_;
	double w_;

	Vec4 normalize() {
		return *this * (1 / length());
	}

	double length() {
		return sqrt(x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_);
	}

	Vec4 operator-() {
		return Vec4(-x_, -y_, -z_, -w_);
	}

	Vec4 operator-(const Vec4 & rhs) {
		return Vec4(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_, w_ - rhs.w_);
	}

	double operator*(const Vec4 & rhs) {
		return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_ + w_ * rhs.w_;
	}

	Vec4 operator*(const double& rhs) {
		return Vec4(x_ * rhs, y_ * rhs, z_ * rhs, w_ * rhs);
	}

	Vec4 operator+(const Vec4 & rhs) {
		return Vec4(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_ , w_ + rhs.w_);
	}

	Vec4& operator+=(const Vec4& rhs) {
		x_ += rhs.x_;
		y_ += rhs.y_;
		z_ += rhs.z_;
		w_ += rhs.w_;
		return *this;
	}

	Vec4& operator=(const Vec4& rhs) {
		x_ = rhs.x_;
		y_ = rhs.y_;
		z_ = rhs.z_;
		w_ = rhs.w_;
		return *this;
	}

	//计算光线与材质颜色总和时使用
	Vec4 operator&(const Vec4& rhs) {
		return Vec4(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_, w_ * rhs.w_);
	}

	//对颜色进行规约化
	Vec4& regularize() {
		if (this->x_ > 1.0) x_ = 1.0;
		if (this->y_ > 1.0) y_ = 1.0;
		if (this->z_ > 1.0) z_ = 1.0;
		if (this->w_ > 1.0) w_ = 1.0;
		return *this;
	}
};

struct Ray
{
	Vec3 e;
	Vec3 d;
};

struct Material;

struct HitRecord
{
	double t;
	Vec3 normal;
	std::shared_ptr<Material> mat;
};