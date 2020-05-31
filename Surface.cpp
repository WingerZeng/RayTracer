#include "Surface.h"
#include <algorithm>
#include <math.h>

Node::Node()
{
}


Node::~Node()
{
}

int Node::setMaterial(std::shared_ptr<Material> Mat)
{
	mat = Mat;
	hasMat = true;
	return 0;
}

std::shared_ptr<Material> Node::getMaterial()
{
	return mat;
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
		rec->normal = ((ray.e + ray.d * t) - center_).normalize();
		//std::cout << rec->normal;
		rec->mat = getMaterial();
	}
	return true;
}

Ground::Ground(double y)
	:y_(y)
{
}

bool Ground::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.y_ > y_ && ray.d.y_ < 0) {
		double t = (ray.e.y_ - y_) / -ray.d.y_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		rec->normal = Vec3(0, 1, 0);
		rec->mat = this->getMaterial();
		return true;
	}
	else return false;
}

void Group::addChild(std::shared_ptr<Node> child)
{
	children.push_back(child);
}

bool Group::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	bool flag = 0;
	double t = t1;
	for (const auto& child : children) {
		double tempt;
		HitRecord temprec;
		if (child->hit(ray, t0, t1, &temprec)) {
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
}

bool Wall_z::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.z_ > z_ && ray.d.z_ < 0) {
		double t = (ray.e.z_ - z_) / -ray.d.z_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		rec->normal = Vec3(0, 0, 1);
		rec->mat = this->getMaterial();
		return true;
	}
	else return false;
	return false;
}

Wall_x::Wall_x(double x)
	:x_(x)
{
}

bool Wall_x::hit(Ray ray, double t0, double t1, HitRecord* rec)
{
	if (ray.e.x_ > x_ && ray.d.x_ < 0) {
		double t = (ray.e.x_ - x_) / -ray.d.x_;
		if (t<t0 || t>t1) return false;
		rec->t = t;
		rec->normal = Vec3(1, 0, 0);
		rec->mat = this->getMaterial();
		return true;
	}
	else return false;
}

Material::Material(Color amb, Color dif, Color spe, double shi)
	:ambient(amb),diffuse(dif),specular(spe),shine(shi)
{
	type = NORMAL;
}

Material::Material(Color amb, Color dif, Color spe, Color mir, double shi)
	:ambient(amb), diffuse(dif), specular(spe),mirror(mir),shine(shi)
{
	type = Type(SPECULAR|NORMAL);
}

Material::Material(Color a, double nr)
	:a_(a),nr_(nr)
{
	type = TRANSPARENT;
}
