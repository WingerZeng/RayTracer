#pragma once
#include "types.h"
#include "RTObject.h"
class Camera: public RTObject
{
public:
	//lookFrom为原点，lookAt为窗口的中心点，vup为全局上方向，vfov为u方向视角，aspect为宽高比
	Camera(Vec3 lookFrom,Vec3 lookAt, Vec3 vup, double vfov ,double aspect);
	Camera(const Camera& camera, rt::CopyOp copyop);
	META_Object(Camera);

	// s = 0~1  t = 0~1
	virtual Ray getRay(float s, float t); 
	virtual ~Camera();

protected:
	void init(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect);

	Vec3 u_;
	Vec3 v_;
	Vec3 w_;
	Vec3 e_;
	double d_;
	double width;
	double height;
	Vec3 leftLowerCorner;
	Vec3 lookAt_;
};

class ChangeFovCamera : public Camera
{
public:
	ChangeFovCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect);
	ChangeFovCamera(const ChangeFovCamera& camera, rt::CopyOp copyop);
	META_Object(ChangeFovCamera);
protected:
	void timeChanged(double time) override;

	double originWidth;
	double originHeight;
};

class DepthCamera: public Camera //景深相机
{
public:
	//lookAt控制焦距，aperture控制光圈大小
	DepthCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect, double aperture);
	DepthCamera(const DepthCamera & camera, rt::CopyOp copyop);
	META_Object(DepthCamera);

	// s = 0~1  t = 0~1
	Ray getRay(float s, float t) override;

private:
	double aperture_;
};
