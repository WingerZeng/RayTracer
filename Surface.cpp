#include "Surface.h"
#include <algorithm>
#include <math.h>
using namespace rt;

Node::Node()
{
}

Node::~Node()
{
}

Vec3 Node::getNormal(Vec3 pos)
{
	if (getMaterial()) {
		return getMaterial()->getNormal(getRawNormal(pos));
	}
	else {
		return getRawNormal(pos);
	}
	return Vec3();
}

int Node::setMaterial(std::shared_ptr<Material> Mat)
{
	mat = Mat;
	hasMat = true;
	RTObject::addChild(Mat);
	return 0;
}

void Node::doAfterHit(Ray ray, HitRecord * rec)
{
}

Sphere::Sphere(const Sphere & sphere, rt::CopyOp copyop)
	:Drawable(sphere,copyop),center_(sphere.center_),radius_(sphere.radius_)
{
}

Vec3 Sphere::getRawNormal(Vec3 pos)
{
	return (pos - center_).normalize();
}

bool Sphere::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	double dieta = pow(ray.d * (ray.e - center_), 2) - (ray.d * ray.d) * ((ray.e - center_) * (ray.e - center_) - radius_ * radius_);
	double deno = ray.d * ray.d;
	double pre = -ray.d * (ray.e - center_);
	double t;
	if (abs(dieta) < ZERO) {
		t = pre / deno;
	}
	else if (dieta > 0) {
		double pt = (pre - sqrt(dieta)) / deno;
		double nt = (pre + sqrt(dieta)) / deno;
		bool f1 = false;
		bool f2 = false;
		if (pt > t1 || pt < t0) f1 = true;
		if (nt > t1 || nt < t0) f2 = true;

		//Vec3 p1 = ray.e + ray.d * pt;
		//Vec3 p2 = ray.e + ray.d * nt;

		if (f1 && f2) {
			return false;
		}
		if (!f1) {
			t = pt;
		}
		else {
			t = nt;
		}
	}
	else if (dieta < 0) {
		return false;
	}
	if (rec) {
		rec->t = t;
		//std::cout << rec->normal;
	}
	return true;
}

Ground::Ground(double y)
	:y_(y)
{
	type = GROUND;
}

Ground::Ground(const Ground & ground, rt::CopyOp copyop)
	: Drawable(ground, copyop),y_(ground.y_)
{
}

bool Ground::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.y_ > y_ && ray.d.y_ < 0) {
		double t = (ray.e.y_ - y_) / -ray.d.y_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		return true;
	}
	else return false;
}

Group::Group(const Group & group, rt::CopyOp copyop)
	:Node(group,copyop)
{
	switch (copyop)
	{
	case rt::SHALLOW_COPY:
		break;
	case rt::RECUR_SHALLOW_COPY:
		for (const auto& child : group.children) {
			addChild(child->cloneToSharedPtr(RECUR_SHALLOW_COPY));
		}
		break;
	case rt::DEEP_COPY:
		break;
	default:
		break;
	}
}

void Group::addChild(std::shared_ptr<Node> child)
{
	children.push_back(child);
	RTObject::addChild(child);
}

bool Group::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	bool flag = 0;
	double t = t1;
	for (const auto& child : children) {
		double tempt;
		HitRecord temprec;
		if (child->calHit(ray, t0, t1, &temprec)) { //此处调用calHit，会导致重复计算纹理位置
			flag = true;
			if(rec) *rec = temprec;
			t1 = temprec.t;
		}
	}
	if (flag) return true;
	else return false;
}

Wall_z::Wall_z(double z)
	:z_(z)
{
	type = WALL_Z;
}

Wall_z::Wall_z(const Wall_z & wall, rt::CopyOp copyop)
	:Drawable(wall, copyop),z_(wall.z_)
{
}

bool Wall_z::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.z_ > z_ && ray.d.z_ < 0) {
		double t = (ray.e.z_ - z_) / -ray.d.z_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		return true;
	}
	else return false;
	return false;
}

Wall_x::Wall_x(double x)
	:x_(x)
{
	type = WALL_X;
}

Wall_x::Wall_x(const Wall_x & wall, rt::CopyOp copyop)
	:Drawable(wall, copyop),x_(wall.x_)
{
}

bool Wall_x::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.x_ > x_ && ray.d.x_ < 0) {
		double t = (ray.e.x_ - x_) / -ray.d.x_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		return true;
	}
	else return false;
}

void Drawable::doAfterHit(Ray ray, HitRecord * rec)
{
	rec->normal = getNormal(ray.e + ray.d * rec->t);
}

HeartShape::HeartShape(const HeartShape & heartshape, rt::CopyOp copyop)
	:Drawable(heartshape,copyop),center_(heartshape.center_),scale_(heartshape.scale_)
{

}

Vec3 HeartShape::getRawNormal(Vec3 pos)
{
	//梯度方向
	Vec3 n;
	pos = (pos - center_) * (1.0/scale_);
	double x = pos.x_;
	double y = pos.z_;
	double z = pos.y_;
	double temp = 3*pow(x * x + 9.0 / 4 * y*y + z * z - 1,2);
	n.x_ = -2 * x*z*z*z + temp * 2 * x;
	n.y_ = 9.0 / 40 * y*z*z*z + temp * 9.0/2 * y;
	n.z_ = -3 * x*x*z*z - 27.0 / 80 * y*y*z*z + temp * z;
	return n.normalize();
}

bool HeartShape::hit(Ray ray, double t0, double t1, HitRecord * rec)
{
	double root;
	double dist = (center_ - ray.e).length();
	ray.d = ray.d * (1.0/scale_);
	ray.e = (ray.e - center_) * (1.0 / scale_);

	//static ImplicitSurfaceOctree* ptree = nullptr;
	//if (!ptree) {
	//	Vec3 bound[2];
	//	bound[0] = Vec3(-2, -2, -2);
	//	bound[1] = Vec3(2, 2, 2);
	//	ptree = new ImplicitSurfaceOctree(bound, std::bind(&HeartShape::heartImplicitFunc, std::placeholders::_1));
	//	ptree->build();
	//}

 //	return ptree->hit(ray, t0, t1, rec);
	auto f = std::bind(&HeartShape::heartFunc<algorithm::IntervalArith>, std::placeholders::_1, ray);
	auto fd = std::bind(&HeartShape::heartFuncd<algorithm::IntervalArith>, std::placeholders::_1, ray);
	double dist1 = ((ray.e).length()-2) / (ray.d).length();
	double dist2 = ((ray.e).length()+2) / (ray.d).length();
	dist1 = std::max(dist1,t0);
	dist2 = std::min(dist2, t1);

	if (algorithm::calMinRoots(f, fd, dist1, dist2, &root) == 0 && (root>=t0&&root<=t1)) {
		rec->t = root;
		return true;
	}
	return false;
}
