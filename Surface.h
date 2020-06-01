#pragma once
#include <vector>
#include "types.h"
#include "definitions.h"
#include "RTObject.h"

class Material
{
public:
	Material(Color amb, Color dif, Color spe, double shi); //NORMAL MERERIAL
	Material(Color amb, Color dif, Color spe, Color mir, double shi); //SPECULAR MERERIAL
	Material(Color a, double nr); //TRANSPARENT MERERIAL
	Material() = default;
	enum Type {
		NORMAL = 0x1,
		SPECULAR = 0x2,
		TRANSPARENT = 0x4,
	};

	//virtual Type getType() { return type; }
	//virtual Color getAmbient() { return ambient; }
	//virtual Color getDiffuse() { return diffuse; }
	//virtual Color getSpecular() { return specular; }
	//virtual Color getMirror() { return mirror; }
	//virtual double getShine() { return shine; }
	virtual void setMirrorBlur(double b) { blur = b; }
	//virtual Color getRefraction() { return refraction; }
	//virtual double getRefraCoef() { return nr; }
	//virtual Color getAttenuation() { return a; }

	virtual Type getType() { return type; }
	virtual Color getAmbient() { return ambient; }
	virtual Color getDiffuse() { return diffuse; }
	virtual Color getSpecular() { return specular; }
	virtual Color getMirror() { return mirror; }
	virtual double getShine() { return shine; }
	virtual double getMirrorBlur() { return blur; }
	virtual double getRefraCoef() { return nr_; }
	virtual Color getAttenuation() { return a_; }

private:
	Type type;
	Color ambient;
	Color diffuse;
	Color specular;
	Color mirror;
	double shine;
	double blur=0;
	//for refraction
	Color a_; //¹âÇ¿ËðÊ§
	double nr_; //refraction coefficient
};
class Node:public RTObject
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
	virtual ~Node();
	virtual bool hit(Ray ray,double t0,double t1,HitRecord* rec) = 0;
	int setMaterial(std::shared_ptr<Material> Mat);
	std::shared_ptr<Material> getMaterial();

protected:
	Type type;
private:
	bool hasMat = false;
	std::shared_ptr<Material> mat;
};

class Group :public Node
{
public:
	void addChild(std::shared_ptr<Node> child);
	bool hit(Ray ray, double t0, double t1, HitRecord* rec) override;
private:
	std::vector<std::shared_ptr<Node>> children;
};

class Sphere : public Node
{
public:
	Sphere(Vec3 center,double radius)
		:center_(center),radius_(radius) {
		type = SPHERE;
	}

	bool hit(Ray ray, double t0, double t1, HitRecord* rec) override;

private:
	Vec3 center_;
	double radius_;
};

class Ground : public Node
{
public:
	Ground(double y);

	bool hit(Ray ray, double t0, double t1, HitRecord* rec) override;
private:
	double y_;
};

class Wall_z : public Node
{
public:
	Wall_z(double z);

	bool hit(Ray ray, double t0, double t1, HitRecord* rec) override;
private:
	double z_;
};

class Wall_x : public Node
{
public:
	Wall_x(double x);

	bool hit(Ray ray, double t0, double t1, HitRecord* rec) override;
private:
	double x_;
};

//__global__ void hit(Node* node, Ray ray, double t0, double t1, HitRecord* rec, bool* hit) {
//	switch (node->type)
//	{
//	case Node::SPHERE:
//	{
//		double dieta = pow(ray.d * (ray.e - center_), 2) - (ray.d * ray.d) * ((ray.e - center_) * (ray.e - center_) - radius_ * radius_);
//		double deno = ray.d * ray.d;
//		double pre = -ray.d * (ray.e - center_);
//		double t;
//		if (abs(dieta) < ZERO) {
//			t = pre / deno;
//		}
//		else if (dieta > 0) {
//			double pt = (pre - sqrt(dieta)) / deno;
//			double nt = (pre + sqrt(dieta)) / deno;
//			bool f1 = false;
//			bool f2 = false;
//			if (pt > t1 || pt < t0) f1 = true;
//			if (nt > t1 || nt < t0) f2 = true;
//
//			//Vec3 p1 = ray.e + ray.d * pt;
//			//Vec3 p2 = ray.e + ray.d * nt;
//
//			if (f1 && f2) {
//				return false;
//			}
//			if (!f1) {
//				t = pt;
//			}
//			else {
//				t = nt;
//			}
//		}
//		else if (dieta < 0) {
//			return false;
//		}
//		if (rec) {
//			rec->t = t;
//			rec->normal = ((ray.e + ray.d * t) - center_).normalize();
//			//std::cout << rec->normal;
//			rec->mat = getMaterial();
//		}
//
//	}
//	default:
//		break;
//	}
//}