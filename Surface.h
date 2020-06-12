#pragma once
#include <vector>
#include "types.h"
#include "definitions.h"
#include "RTObject.h"
#include "algorithms.h"
#include "octree.h"
#include "material.h"
using namespace rt;
class AbstructNode
{
public:
	int getBoundBox(Vec3 box[2]);
	virtual int computeBoundBox() {}; //TODO 完成包围盒计算
	virtual bool calHit(const Ray& ray, double t0, double t1, HitRecord* rec) = 0;
	inline bool isHitBox(const Ray& ray, double t0, double t1) { return algorithm::hitBox(box_, ray, t0, t1, nullptr); }

protected:
	bool dirtyBound_ = true;
	Vec3 box_[2];
};

class Node:public AbstructNode,public RTObject //用visitor模式来实现hit
{
public:
	enum Type {
		GROUP = 0,
		SPHERE,
		GROUND,
		WALL_Z,
		WALL_X,
	};
	Node();
	Node(const Node& node, rt::CopyOp copyop);
	META_Object(Node)
	virtual ~Node();
	bool calHit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
	virtual Vec3 getNormal(Vec3 pos);
	int setMaterial(std::shared_ptr<Material> Mat);
	std::shared_ptr<Material> getMaterial() const { return mat; }

protected:
	virtual Vec3 getRawNormal(Vec3 pos) { return Vec3(1,0,0); }
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) { return false; };
	virtual void doAfterHit(const Ray& ray, HitRecord* rec);
	Type type;
private:
	bool hasMat = false;
	std::shared_ptr<Material> mat;
};

class Group :public Node
{
public:
	Group() = default;
	Group(const Group& group, rt::CopyOp copyop);
	META_Object(Group)
	void addChild(std::shared_ptr<Node> child);
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
private:
	std::vector<std::shared_ptr<Node>> children;
};

class Drawable : public Node
{
public:
	using Node::Node;
	void doAfterHit(const Ray& ray, HitRecord * rec);
};

class Sphere : public Drawable
{
public:
	Sphere(Vec3 center,double radius)
		:center_(center),radius_(radius) {
		type = SPHERE;
	}
	Sphere(const Sphere& sphere, rt::CopyOp copyop);

	META_Object(Sphere)

	Vec3 getRawNormal(Vec3 pos) override;
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
private:
	Vec3 center_;
	double radius_;
};

class HeartShape : public Drawable
{
public:
	HeartShape(Vec3 center, double scale)
		:center_(center), scale_(scale) {};
	HeartShape(const HeartShape& heartshape, rt::CopyOp copyop);

	META_Object(HeartShape)

	Vec3 getRawNormal(Vec3 pos) override;
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
private:
	template<typename T> static T heartFunc(T in,Ray ray);
	template<typename T> static T heartFuncd(T in,Ray ray);
	inline static double heartImplicitFunc(Vec3 pos);
	Vec3 center_;
	double scale_;
};

class Ground : public Drawable
{
public:
	Ground(double y);
	Ground(const Ground& ground, rt::CopyOp copyop);
	META_Object(Ground)

	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
	Vec3 getRawNormal(Vec3 pos) override { return Vec3(0, 1, 0); }
private:
	double y_;
};

class Wall_z : public Drawable
{
public:
	Wall_z(double z);
	Wall_z(const Wall_z& wall, rt::CopyOp copyop);
	META_Object(Wall_z)

	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
	Vec3 getRawNormal(Vec3 pos) override { return Vec3(0, 0, 1); };
private:
	double z_;
};

class Wall_x : public Drawable
{
public:
	Wall_x(double x);
	Wall_x(const Wall_x& wall, rt::CopyOp copyop);

	META_Object(Wall_x)

	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
	Vec3 getRawNormal(Vec3 pos) override { return Vec3(1, 0, 0); };
private:
	double x_;
};

//--------inline definition-----------------

inline double HeartShape::heartImplicitFunc(Vec3 pos) {
	double x = pos.x_;
	double y = pos.z_;
	double z = pos.y_;
	double temp = x * x + y * y * 9.0 / 4 + z * z - 1;
	return -(x*x*z*z*z) - y * y*z*z*z*9.0 / 80 + temp * temp*temp;
}

template<typename T>
inline T HeartShape::heartFunc(T in, Ray ray)
{
	const auto& d = ray.d;
	const auto& e = ray.e;
	//交换xy轴
	T x = in * d.x_ + e.x_;
	T y = in * d.z_ + e.z_;
	T z = in * d.y_ + e.y_;
	T temp = x * x + y * y * 9.0 / 4 + z * z - 1;
	return -(x*x*z*z*z) - y * y*z*z*z*9.0 / 80 + temp*temp*temp;
}

template<typename T>
inline T HeartShape::heartFuncd(T in, Ray ray)
{
	const auto& d = ray.d;
	const auto& e = ray.e;
	T x = in * d.x_ + e.x_;
	T y = in * d.z_ + e.z_;
	T z = in * d.y_ + e.y_;
	T inpow = x * x + y * y * 9.0 / 4 + z * z - 1;
	T temp = inpow*inpow*3;
	T nx = x*z*z*z*-2 - z * y*y*z*z*9.0 / 80 + temp * 2 * x;
	T ny = x * x*z*z*z -  z* y*z*z*9.0 / 40 + temp * 9.0 / 2 * y;
	T nz =  x*x*z*z *-3 - z * y*y*z*27.0 / 80 + temp * z;
	return nx * d.x_ + ny * d.y_ + nz * d.z_;
}

inline Node::Node(const Node & node, rt::CopyOp copyop)
	:RTObject(node,copyop)
{
	switch (copyop)
	{
	case rt::SHALLOW_COPY:
		break;
	case rt::RECUR_SHALLOW_COPY:
		if(node.getMaterial()) setMaterial(node.mat->cloneToSharedPtr(RECUR_SHALLOW_COPY));
		break;
	case rt::DEEP_COPY:
		break;
	default:
		break;
	}
}

inline bool Node::calHit(const Ray& ray, double t0, double t1, HitRecord * rec)
{
	if (hit(ray, t0, t1, rec)) {
		if (mat) {
			rec->mat = mat;
			mat->setPosition(ray.d * rec->t + ray.e); //TODO position不能作为状态，而应该是作为传入参数
		}
		doAfterHit(ray, rec);   //TODO 这个后处理应该在顶层做，因为顶层的效果可能会覆盖底层的效果，不能让底层的效果先做掉
		return true;
	}
	else return false;
}