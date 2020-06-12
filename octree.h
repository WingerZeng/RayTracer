#pragma once
#include <vector>
#include <list>
#include "types.h"
#include "definitions.h"
#include "Surface.h"
class OctreeNode {
public:
	OctreeNode(Vec3 bound[2], int maxSize);
	virtual ~OctreeNode();
	inline Vec3 getPoint(int index);
	inline Vec3 getSize();
	virtual bool hit(const Ray& ray, double t0,double t1, HitRecord* rec);
	bool isLeaf();
	std::shared_ptr<OctreeNode> getChild(int index) { return children_[index]; }
	void getChildBound(int childindex, Vec3 bound[2]);

	//不会检查对象在该Node中是否合理，需要在插入前检查
	void insertObject(std::shared_ptr<AbstructNode> objects_);
protected:
	void createChildren();

	static const int OverlapTolerance = 2; //如果目标的包围盒覆盖超过OverlapTolerance个子节点，则目标不被保存进子节点，而存在该节点中

	Vec3 bound_[2];
	unsigned int maxSize_;
	std::shared_ptr<OctreeNode> children_[8];
	bool full = false;
	std::list<std::shared_ptr<AbstructNode>> objects_;
};

//class ImplicitSurfaceOctreeNode : public OctreeNode
//{
//public:
//	ImplicitSurfaceOctreeNode(Vec3 bound[2], StdImpicitFunc func);
//	void addChild(int index, std::shared_ptr<ImplicitSurfaceOctreeNode> child);
//	std::shared_ptr<ImplicitSurfaceOctreeNode> getChild(int index);
//	inline bool isIntersectSurface();
//protected:
//	std::shared_ptr<ImplicitSurfaceOctreeNode> impchildren_[8];
//	StdImpicitFunc sfunc_;
//};

class Octree 
{
public:
	Octree(int maxSize = 64);
	virtual ~Octree() {};
	virtual bool hit(const Ray& ray, double t0, double t1, HitRecord* rec);
protected:
	unsigned int maxSize_;
	std::shared_ptr<OctreeNode> root_;
};

class ImplicitSurfaceOctree : Octree 
{
public:
	ImplicitSurfaceOctree(Vec3 bound[2], StdImpicitFunc func);
	inline int setMinSize(double minsize) { minSize_ = minsize; };
	virtual bool hit(const Ray& ray, double t0, double t1, HitRecord* rec);
	void build();
protected:
	bool includeSurface_ = true;
	double minSize_ = 10e-8;
	Vec3 bound_[2];
	StdImpicitFunc sfunc_;
};
/*------------inline def------------------*/

inline Vec3 OctreeNode::getSize() {
	return bound_[1] - bound_[0];
}

inline void OctreeNode::getChildBound(int childindex, Vec3 bound[2])
{
	if (childindex < 0) return;
	switch (childindex%8)
	{
	case 0:
		bound[0] = getPoint(0);
	    bound[1] = (getPoint(0)+ getPoint(6))/2;
		break;
	case 1:
		bound[0] = (getPoint(0) + getPoint(1))/2;
		bound[1] = (getPoint(1) + getPoint(6)) / 2;
		break;
	case 2:
		bound[0] = (getPoint(0) + getPoint(2)) / 2;
		bound[1] = (getPoint(2) + getPoint(6)) / 2;
		break;
	case 3:
		bound[0] = (getPoint(0) + getPoint(3)) / 2;
		bound[1] = (getPoint(3) + getPoint(6)) / 2;
		break;
	case 4:
		bound[0] = (getPoint(0) + getPoint(4)) / 2;
		bound[1] = (getPoint(5) + getPoint(7)) / 2;
		break;
	case 5:
		bound[0] = (getPoint(1) + getPoint(4)) / 2;
		bound[1] = (getPoint(5) + getPoint(6)) / 2;
		break;
	case 6:
		bound[0] = (getPoint(1) + getPoint(7)) / 2;
		bound[1] = getPoint(6);
		break;
	case 7:
		bound[0] = (getPoint(0) + getPoint(7)) / 2;
		bound[1] = (getPoint(6) + getPoint(7)) / 2;
		break;
	}
}


inline Vec3 OctreeNode::getPoint(int index)
{
	return algorithm::getBoundPoint(index, bound_);
}

//inline bool ImplicitSurfaceOctreeNode::isIntersectSurface()
//{
//	double ret;
//	bool pflag = 1;
//	bool nflag = 1;
//	for (int i = 0; i < 8; i++) {
//		ret = sfunc_(getPoint(i));
//		if (ret > 0) nflag = false;
//		if (ret < 0) pflag = false;
//	}
//	if (nflag || pflag)
//		return false;
//	else
//		return true;
//}