#pragma once
#include "types.h"
class Camera
{
public:
	//lookFromΪԭ�㣬lookAtΪ���ڵ����ĵ㣬vupΪȫ���Ϸ���vfovΪu�����ӽǣ�aspectΪ��߱�
	Camera(Vec3 lookFrom,Vec3 lookAt, Vec3 vup, double vfov ,double aspect);

	// s = 0~1  t = 0~1
	virtual Ray getRay(float s, float t); 
	virtual ~Camera();

protected:
	Vec3 u_;
	Vec3 v_;
	Vec3 w_;
	Vec3 e_;
	double d_;
	double width;
	double height;
	Vec3 leftLowerCorner;
};

class DepthCamera: public Camera //�������
{
public:
	//lookAt���ƽ��࣬aperture���ƹ�Ȧ��С
	DepthCamera(Vec3 lookFrom, Vec3 lookAt, Vec3 vup, double vfov, double aspect, double aperture);

	// s = 0~1  t = 0~1
	Ray getRay(float s, float t) override;

private:
	double aperture_;
};