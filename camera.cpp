#include "time.h"
#include "camera.h"
#include "definitions.h"
#include "algorithms.h"
Camera::Camera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect)
{
	d_ = (lookAt - lookFrom).length();
	w_ = (lookFrom - lookAt).normalize();
	u_ = (vup ^ w_).normalize();
	v_ = (w_^u_).normalize();
	e_ = lookFrom;
	height = tan(vfov * PI / 180 / 2) * d_ * 2;
	width = height * aspect;
	leftLowerCorner = lookAt - v_ * (height/2) - u_ * (width/2);
}

Ray Camera::getRay(float s, float t)
{
	Ray ray;
	ray.e = e_;
	Vec3 lookat = leftLowerCorner + u_ * s * width + v_ * t * height;
	ray.d = (lookat - e_).normalize();
	return ray;
}

Camera::~Camera()
{
}

DepthCamera::DepthCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect, double aperture)
	:Camera(lookFrom, lookAt, vup, vfov, aspect),aperture_(aperture)
{
	setRandomEnabled(true);
}

Ray DepthCamera::getRay(float s, float t)
{
	// TODO 改造为基于圆盘的随机
	Ray ray;
	double x, y;
	algorithm::randFromDisk(&x, &y);
	ray.e = e_ + u_ * (x * aperture_) + v_ * (y * aperture_);
	Vec3 lookat = leftLowerCorner + u_ * s * width + v_ * t * height;
	ray.d = (lookat - ray.e).normalize();
	return ray;
}
