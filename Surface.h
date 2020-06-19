#pragma once
#include <vector>
#include "types.h"
#include "definitions.h"
#include "RTObject.h"
#include "algorithms.h"
#include "octree.h"
#include "material.h"
using namespace rt;
class ImplicitSurfaceOctree; 
template<class ImpFunc> class ImplicitSurface;

class AbstructNode
{
public:
	int getBoundBox(Vec3 box[2]);
	virtual int computeBoundBox() { return 0; }; //TODO 完成包围盒计算
	virtual bool calHit(const Ray& ray, double t0, double t1, HitRecord* rec) = 0;
	inline bool isHitBox(const Ray& ray, double t0, double t1) { return algorithm::hitBox(box_, ray, t0, t1, nullptr); }

protected:
	bool dirtyBound_ = true;
	Vec3 box_[2];
};

class BoxNode :public AbstructNode {
public:
	BoxNode(BoundBox_t bound);
	virtual bool calHit(const Ray & ray, double t0, double t1, HitRecord * rec) override;
	bool operator<(const BoxNode& rhs) const { //根据两个包围盒的p0来比大小，用于set等标准库容器
		const auto& p1 = box_[0];
		const auto& p2 = rhs.box_[0];
		bool ret;
		if ((p1 - p2).length() < ZERO) ret = false;
		else
		{
			abs(p1.x_ - p2.x_) > ZERO ? ret = p1.x_ < p2.x_ :
				abs(p1.y_ - p2.y_)>ZERO ? ret = p1.y_ < p2.y_ :
				abs(p1.z_ - p2.z_)>ZERO ? ret = p1.z_ < p2.z_ :
				ret = false;
		}
		//std::cout << p1 << "  " << p2 << "  " << ret << std::endl;
		return ret;
	}
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
	virtual bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) { return false; };
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

class AbstuctImplicitFunction {
public:
	virtual double ImplicitFunc(Vec3 pos) = 0;
	virtual double getPrecision() { return 5e-3; };
	virtual Vec3 getSeedPoint(int index) = 0;
	virtual Vec3 getNormal(Vec3 pos) = 0;
};

class SurfaceOfGenus2Function : public AbstuctImplicitFunction {
public:
	double getPrecision() override { return 3e-3; }
	double ImplicitFunc(Vec3 pos) override;
	Vec3 getSeedPoint(int index) override;
	Vec3 getNormal(Vec3 pos) override;
};

typedef ImplicitSurface<SurfaceOfGenus2Function> SurfaceOfGenus2;

template<class ImpFunc>
class ImplicitSurface :public Drawable
{
public:
	ImplicitSurface(Vec3 center, double scale);
	ImplicitSurface(const ImplicitSurface& surface, rt::CopyOp copyop);

	META_Object(ImplicitSurface<ImpFunc>)

	Vec3 getRawNormal(Vec3 pos) override;
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
private:
	void buildTree();
	static ImplicitSurfaceOctree* ptree;
	Vec3 center_;
	double scale_;
	ImpFunc func;
};
template<class ImpFunc> ImplicitSurfaceOctree* ImplicitSurface<ImpFunc>::ptree=nullptr;

class HeartShape : public Drawable
{
public:
	HeartShape(Vec3 center, double scale)
		:center_(center), scale_(scale) {
		buildTree();
	};
	HeartShape(const HeartShape& heartshape, rt::CopyOp copyop);

	META_Object(HeartShape)

	Vec3 getRawNormal(Vec3 pos) override;
	bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;

	static ImplicitSurfaceOctree* ptree;
private:
	void buildTree();

	template<typename T> static T heartFunc(T in,Ray ray);
	//template<typename T> static T heartFuncd(T in,Ray ray);
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

//template<typename T>
//inline T HeartShape::heartFuncd(T in, Ray ray)
//{
//	const auto& d = ray.d;
//	const auto& e = ray.e;
//	T x = in * d.x_ + e.x_;
//	T y = in * d.z_ + e.z_;
//	T z = in * d.y_ + e.y_;
//	T inpow = x * x + y * y * 9.0 / 4 + z * z - 1;
//	T temp = inpow*inpow*3;
//	T nx = x*z*z*z*-2 - z * y*y*z*z*9.0 / 80 + temp * 2 * x;
//	T ny = x * x*z*z*z -  z* y*z*z*9.0 / 40 + temp * 9.0 / 2 * y;
//	T nz =  x*x*z*z *-3 - z * y*y*z*27.0 / 80 + temp * z;
//	return nx * d.x_ + ny * d.y_ + nz * d.z_;
//}

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
		if (mat&&rec) {
			rec->mat = mat;
			mat->setPosition(ray.d * rec->t + ray.e); //TODO position不能作为状态，而应该是作为传入参数
		}
		doAfterHit(ray, rec);   //TODO 这个后处理应该在顶层做，因为顶层的效果可能会覆盖底层的效果，不能让底层的效果先做掉
		return true;
	}
	else return false;
}

template<class ImpFunc>
inline ImplicitSurface<ImpFunc>::ImplicitSurface(Vec3 center, double scale)
	:center_(center),scale_(scale)
{
	buildTree();
}

template<class ImpFunc>
inline ImplicitSurface<ImpFunc>::ImplicitSurface(const ImplicitSurface & surface, rt::CopyOp copyop)
	:Drawable(surface, copyop), center_(surface.center_), scale_(surface.scale_)
{
	buildTree();
}

template<class ImpFunc>
inline Vec3 ImplicitSurface<ImpFunc>::getRawNormal(Vec3 pos)
{
	pos = (pos - center_) * (1.0 / scale_);
	return func.getNormal(pos);
}

template<class ImpFunc>
inline bool ImplicitSurface<ImpFunc>::hit(const Ray & ray, double t0, double t1, HitRecord * rec)
{
	Ray newray;
	newray.d = ray.d * (1.0 / scale_);
	newray.e = (ray.e - center_) * (1.0 / scale_);

	return ptree->hit(newray, t0, t1, rec);
}

template<class ImpFunc>
inline void ImplicitSurface<ImpFunc>::buildTree()
{
	//使用八叉树空间分片形式求交点
	if (!ptree) {
		Point_t seedp[2]{ func.getSeedPoint(0),func.getSeedPoint(1) };
		ptree = new ImplicitSurfaceOctree(std::bind(&ImpFunc::ImplicitFunc, &func, std::placeholders::_1), seedp, func.getPrecision());
		ptree->build();
	}
}
