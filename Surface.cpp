#include "Surface.h"
#include <algorithm>
#include <math.h>
#include "octree.h"
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
}

int Node::setMaterial(std::shared_ptr<Material> Mat)
{
	mat = Mat;
	hasMat = true;
	RTObject::addChild(Mat);
	return 0;
}

void Node::doAfterHit(const Ray& ray, HitRecord * rec)
{
}

BoxNode::BoxNode(BoundBox_t bound)
{
	box_[0] = bound[0];
	box_[1] = bound[1];
}

bool BoxNode::calHit(const Ray & ray, double t0, double t1, HitRecord * rec)
{
	double rt0, rt1;
	if (algorithm::hitBox(box_, ray, t0, t1, &rt0, &rt1)) {
		//rec->t = (rt0 + rt1) / 2;  //不能将中间的点认为是击中点，否则判断阴影的时候会从里面射出来，然后自相交
		//将光线交到立方体的外接球上
		double r = (box_[0] - box_[1]).length() / 2 * 2.5;
		double dist = ((box_[0] + box_[1]) / 2 - ray.e).length();
		double dist2 = ray.d.normalize() * ((box_[0] + box_[1]) / 2 - ray.e);
		double dist3 = sqrt(dist*dist - dist2 * dist2);
		double dist4 = dist2 - sqrt(r*r - dist3 * dist3);
		rec->t = dist4 / ray.d.length();
		rec->t = rt0 - (box_[0] - box_[1]).length() / (ray.d.length());
		return true;
	}
	return false;
}

ImplicitSurfaceOctree* HeartShape::ptree = nullptr;

Sphere::Sphere(const Sphere & sphere, rt::CopyOp copyop)
	:Drawable(sphere,copyop),center_(sphere.center_),radius_(sphere.radius_)
{
}

Vec3 Sphere::getRawNormal(Vec3 pos)
{
	return (pos - center_).normalize();
}

bool Sphere::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
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

bool Ground::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
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

bool Group::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
{
	bool flag = 0;
	double t = t1;
	for (const auto& child : children) {
		if (rec) {
			if (child->calHit(ray, t0, t1, rec)) { //此处调用calHit，会导致重复计算纹理位置
				flag = true;
				t1 = rec->t;
			}
		}
		else {
			HitRecord tempt;
			if (child->calHit(ray, t0, t1, &tempt)) { //此处调用calHit，会导致重复计算纹理位置
				flag = true;
				t1 = tempt.t;
			}
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

bool Wall_z::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
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

bool Wall_x::hit(const Ray& ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.x_ > x_ && ray.d.x_ < 0) {
		double t = (ray.e.x_ - x_) / -ray.d.x_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		return true;
	}
	else return false;
}

void Drawable::doAfterHit(const Ray& ray, HitRecord * rec)
{
	if(rec) rec->normal = getNormal(ray.e + ray.d * rec->t);
}

HeartShape::HeartShape(const HeartShape & heartshape, rt::CopyOp copyop)
	:Drawable(heartshape,copyop),center_(heartshape.center_),scale_(heartshape.scale_)
{
	buildTree();
}

Vec3 HeartShape::getRawNormal(Vec3 pos)
{
	//梯度方向
	//double ds = 1e-6;
	//pos = (pos - center_) * (1.0/scale_);
	//double of = heartImplicitFunc(pos);
	//double xf = heartImplicitFunc(pos+Vec3(ds,0,0));
	//double yf = heartImplicitFunc(pos + Vec3(0,ds,0));
	//double zf = heartImplicitFunc(pos + Vec3(0, 0, ds));

	//n = Vec3((xf - of) / ds, (yf - of) / ds, (zf - of) / ds);

	Vec3 n;
	pos = (pos - center_) * (1.0/scale_);
	double x = pos.x_;
	double y = pos.z_;
	double z = pos.y_;
	double temp = x * x + y * y * 9.0 / 4 + z * z - 1;
	temp = 3 * temp*temp;
	temp = std::max(temp, 0.5); //在z=0时，原函数的梯度为0，0，0,   为了避免该情况，需要提高temp值
	n.x_ = -2 * x*z*z*z + temp * 2 * x;
	n.z_ = - 9.0 / 40 * y*z*z*z + temp * 9.0/2 * y;
	n.y_ = -3 * x*x*z*z - 27.0 / 80 * y*y*z*z + 2 * temp * z;
	if((1 + n.normalize().y_)<0.03 && z>-0.9) 
		std::cout << n << ' ' << pos << ' '<< heartImplicitFunc(Vec3(x,z,y)) << ' ' << temp <<std::endl;
	return n.normalize();
}


bool HeartShape::hit(const Ray& ray, double t0, double t1, HitRecord * rec)
{
	Ray newray;
	newray.d = ray.d * (1.0 / scale_);
	newray.e = (ray.e - center_) * (1.0 / scale_);
	bool ret = ptree->hit(newray, t0, t1, rec);
	//if (ret) std::cout << heartImplicitFunc(newray.e + newray.d * rec->t) << std::endl;
	if (ret) rec->localp = newray.e + newray.d * rec->t;
	return ret;

	//数值计算求交的方式已经废弃

	//Ray newray;
	//newray.d = ray.d * (1.0 / scale_);
	//newray.e = (ray.e - center_) * (1.0 / scale_);
	//double root;
	//auto f = std::bind(&HeartShape::heartFunc<algorithm::IntervalArith>, std::placeholders::_1, newray);
	//auto fd = std::bind(&HeartShape::heartFuncd<algorithm::IntervalArith>, std::placeholders::_1, newray);
	//double dist1 = ((newray.e).length()-2) / (newray.d).length();
	//double dist2 = ((newray.e).length()+2) / (newray.d).length();
	//dist1 = std::max(dist1,t0);
	//dist2 = std::min(dist2, t1);

	//if (algorithm::calMinRoots(f, fd, dist1, dist2, &root) == 0 && (root>=t0&&root<=t1)) {
	//	rec->t = root;
	//	return true;
	//}
	//return false;
}

void HeartShape::buildTree()
{
	//使用八叉树空间分片形式求交点
	if (!ptree) {
		Point_t seedp[2]{ Vec3(0,0,0),Vec3(2,0,0) };
		ptree = new ImplicitSurfaceOctree(HeartShape::heartImplicitFunc, seedp);
		ptree->build();
	}
}

int AbstructNode::getBoundBox(Vec3 box[2])
{
	if (dirtyBound_) {
		computeBoundBox();
	}
	box[0] = box_[0];
	box[1] = box_[1];
	return 0;
}

double SurfaceOfGenus2Function::ImplicitFunc(Vec3 pos)
{
	double x = pos.x_;
	double y = pos.z_;
	double z = pos.y_;
	double temp = x * x + y * y;
	return 2 * y *(y*y - 3 * x*x)*(1 - z * z) + temp * temp - (9 * z*z - 1)*(1 - z * z);
}

Vec3 SurfaceOfGenus2Function::getSeedPoint(int index)
{
	if (index == 0) return Vec3(0, 0, 0);
	else return Vec3(0, 0.9, 0);
}

Vec3 SurfaceOfGenus2Function::getNormal(Vec3 pos)
{
	double x = pos.x_;
	double y = pos.z_;
	double z = pos.y_;
	Vec3 n;
	n.x_ = -12 * x * y * (1 - z * z) + 4 * (x*x + y * y)*x;
	n.z_ = 6 * (1 - z * z)*(y*y - x * x) + 4 * (x*x + y * y)*y;
	n.y_ = 2 * y *(y*y - 3 *x*x)*-2 * z + 36 * z *z *z - 20 * z;
	return n.normalize();
}
