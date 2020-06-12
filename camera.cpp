#include "time.h"
#include "camera.h"
#include "definitions.h"
#include "algorithms.h"
Camera::Camera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect)
{
	init(lookFrom, lookAt, vup, vfov, aspect);
}

Camera::Camera(const Camera & camera, rt::CopyOp copyop)
	:RTObject(camera,copyop)
{
	u_ = camera.u_;
	v_ = camera.v_;
	w_ = camera.w_;
	e_ = camera.e_;
	d_ = camera.d_;
	width = camera.width;
	height = camera.height;
	leftLowerCorner = camera.leftLowerCorner;
	lookAt_ = camera.lookAt_;
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

void Camera::init(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect)
{
	d_ = (lookAt - lookFrom).length();
	w_ = (lookFrom - lookAt).normalize();
	u_ = (vup ^ w_).normalize();
	v_ = (w_^u_).normalize();
	e_ = lookFrom;
	height = tan(vfov * PI / 180 / 2) * d_ * 2;
	width = height * aspect;
	lookAt_ = lookAt;
	leftLowerCorner = lookAt - v_ * (height / 2) - u_ * (width / 2);
}

DepthCamera::DepthCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect, double aperture)
	:Camera(lookFrom, lookAt, vup, vfov, aspect),aperture_(aperture)
{
	setRandomEnabled(true);
}

DepthCamera::DepthCamera(const DepthCamera & camera, rt::CopyOp copyop)
	:Camera(camera,copyop)
{
	aperture_ = camera.aperture_;
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

ChangeFovCamera::ChangeFovCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect)
	:Camera(lookFrom, lookAt, vup, vfov, aspect)
{
	originHeight = height;
	originWidth = width;
}

ChangeFovCamera::ChangeFovCamera(const ChangeFovCamera & camera, rt::CopyOp copyop)
	:Camera(camera,copyop)
{
	originHeight = camera.originHeight;
	originWidth = camera.originWidth;
}

void ChangeFovCamera::timeChanged(double time)
{
	double scale = sqrt(sin(time*2*3.1415926)*1.5 + 1.6)+0.3;
	height = originHeight * scale;
	width = originWidth * scale;
	leftLowerCorner = lookAt_ - v_ * (height / 2) - u_ * (width / 2);
}
