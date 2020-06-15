#pragma once
#include <vector>
#include <list>
#include "types.h"
#include "definitions.h"
#include "algorithms.h"

class AbstructNode;
class OctreeNode {
public:
	OctreeNode(Vec3 bound[2], int maxSize);
	virtual ~OctreeNode();
	inline Vec3 getPoint(int index) const;
	inline void getBound(BoundBox_t bound) const;
	inline Vec3 getSize() const;
	virtual bool hit(const Ray& ray, double t0,double t1, HitRecord* rec) const;
	bool isLeaf() const;
	std::shared_ptr<OctreeNode> getChild(int index) const { return children_[index]; }
	void getChildBound(int childindex, Vec3 bound[2]) const;
	
	//不会检查对象在该Node中是否合理，需要在插入前检查
	void insertObject(std::shared_ptr<AbstructNode> objects_);
protected:
	void createChildren();

	static const int OverlapTolerance = 3; //如果目标的包围盒覆盖超过OverlapTolerance个子节点，则目标不被保存进子节点，而存在该节点中

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
	Octree(int maxSize = 128, BoundBox_t bound = nullptr);
	virtual ~Octree() {};
	int setBoundBox(BoundBox_t bound);
	virtual bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) const;
	virtual int build();
	void insertObject(std::shared_ptr<AbstructNode> objects_);
protected:
	const unsigned int maxSize_;
	BoundBox_t bound_;
	std::shared_ptr<OctreeNode> root_;
};

//TODO 要把Surface离散成三角面片，否则边界有一定厚度
class ImplicitSurfaceOctree : public Octree 
{
public:
	ImplicitSurfaceOctree(StdImpicitFunc func, Point_t seedPoints[2], double minSize=3e-3);
	int build() override;
	//bool hit(const Ray& ray, double t0, double t1, HitRecord* rec) override;
protected:
	bool includeSurface_ = true;
	double minSize_;
	Vec3 bound_[2];
	Point_t seedPoints_[2];
	StdImpicitFunc sfunc_;
};
/*------------inline def------------------*/

inline Vec3 OctreeNode::getSize() const {
	return bound_[1] - bound_[0];
}

inline void OctreeNode::getChildBound(int childindex, Vec3 bound[2]) const
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


inline Vec3 OctreeNode::getPoint(int index) const
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

inline void OctreeNode::getBound(BoundBox_t bound) const
{
	bound[0] = bound_[0];
	bound[1] = bound_[1];
}