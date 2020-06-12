#pragma once
#include <vector>
#include "types.h"
#include "definitions.h"
class OctreeNode {
public:
	OctreeNode(Vec3 bound[2]);
	virtual ~OctreeNode();
	inline Vec3 getPoint(int index);
	static inline Vec3 getBoundPoint(int index, Vec3 bound[2]);
	inline Vec3 getSize();
	virtual bool hit(Ray ray, double t0,double t1, HitRecord* rec);
	static inline bool hit(Vec3 bound[3], Ray ray, double t0, double t1, HitRecord* rec);
	void getChildBound(int childindex, Vec3 bound[2]);
protected:
	Vec3 bound_[2];
	std::shared_ptr<OctreeNode> children_[8];
};

class ImplicitSurfaceOctreeNode : public OctreeNode
{
public:
	ImplicitSurfaceOctreeNode(Vec3 bound[2], StdImpicitFunc func);
	void addChild(int index, std::shared_ptr<ImplicitSurfaceOctreeNode> child);
	std::shared_ptr<ImplicitSurfaceOctreeNode> getChild(int index);
	inline bool isIntersectSurface();
protected:
	std::shared_ptr<ImplicitSurfaceOctreeNode> impchildren_[8];
	StdImpicitFunc sfunc_;
};

class Octree 
{
public:
	virtual ~Octree() {};
	virtual bool hit(Ray ray, double t0, double t1, HitRecord* rec) = 0;
protected:
	std::shared_ptr<OctreeNode> root_;
};

class ImplicitSurfaceOctree : Octree 
{
public:
	ImplicitSurfaceOctree(Vec3 bound[2], StdImpicitFunc func);
	inline int setMinSize(double minsize) { minSize_ = minsize; };
	virtual bool hit(Ray ray, double t0, double t1, HitRecord* rec);
	void build();
protected:
	bool includeSurface_ = true;
	double minSize_ = 10e-8;
	Vec3 bound_[2];
	StdImpicitFunc sfunc_;
};
/*------------inline def------------------*/



inline Vec3 OctreeNode::getBoundPoint(int index, Vec3 bound[2]) {
	int ix;
	int iy;
	switch (index % 4)
	{
	case 0:
		ix = iy = 0;
		break;
	case 1:
		ix = 1;
		iy = 0;
		break;
	case 2:
		ix = iy = 1;
		break;
	case 3:
		ix = 1;
		iy = 0;
		break;
	}
	int iz = (index >> 2) & 1;
	return Vec3(bound[ix].x_, bound[iy].y_, bound[iz].z_);
}

inline Vec3 OctreeNode::getSize() {
	return bound_[1] - bound_[0];
}

inline bool OctreeNode::hit(Vec3 bound[3], Ray ray, double t0, double t1, HitRecord* rec) {
	Vec3 vt0 = (bound[0] - ray.e) / ray.d;
	Vec3 vt1 = (bound[1] - ray.e) / ray.d;
	std::vector<double> tmin{ std::min(vt0.x_,vt1.x_),std::min(vt0.y_,vt1.y_),std::min(vt0.z_,vt1.z_) };
	std::vector<double> tmax{ std::max(vt0.x_,vt1.x_),std::max(vt0.y_,vt1.y_),std::max(vt0.z_,vt1.z_) };
	tmin.push_back(t0);
	tmax.push_back(t1);
	if (*std::max_element(tmin.begin(), tmin.end()) <= *std::max_element(tmin.begin(), tmin.end())) {
		if(rec) rec->t = *std::max_element(tmin.begin(), tmin.end());
		return true;
	}
	return false;
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
	return getBoundPoint(index, bound_);
}

inline bool ImplicitSurfaceOctreeNode::isIntersectSurface()
{
	double ret;
	bool pflag = 1;
	bool nflag = 1;
	for (int i = 0; i < 8; i++) {
		ret = sfunc_(getPoint(i));
		if (ret > 0) nflag = false;
		if (ret < 0) pflag = false;
	}
	if (nflag || pflag)
		return false;
	else
		return true;
}