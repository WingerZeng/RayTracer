#pragma once
#include <vector>
#include "types.h"
#include "definitions.h"


class Material
{
public:
	Material(Color amb, Color dif, Color spe, double shi); //NORMAL MERERIAL
	Material(Color amb, Color dif, Color spe, Color mir, double shi); //SPECULAR MERERIAL

	enum Type {
		NORMAL = 0,
		SPECULAR,
		TRANSPARENT,
	};

	Type type;
	Color ambient;
	Color diffuse;
	Color specular;
	Color mirror;
	double shine;
};

class Node
{
public:
	Node();
	virtual ~Node();
	virtual bool hit(Ray ray,double t0,double t1,HitRecord* rec) = 0;
	int setMaterial(std::shared_ptr<Material> Mat);
	std::shared_ptr<Material> getMaterial();
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
		:center_(center),radius_(radius) {}

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