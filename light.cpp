#include "light.h"
#include "Surface.h"
#include "material.h"
Light::~Light()
{
}

Light::Light(Vec3 pos, Color ambient, Color diffuse, Color specular)
	:pos_(pos),a_(ambient),d_(diffuse),s_(specular)
{
}

Light::Light(const Light & light, rt::CopyOp copyop)
	:RTObject(light,copyop)
{
	pos_ = light.pos_;
	a_ = light.a_;
	d_ = light.d_;
	s_ = light.s_;
}

Color Light::getDiffuse()
{
	return d_;
}

Color Light::getAmbient()
{
	return a_;
}

Color Light::getSpecular()
{
	return s_;
}

Vec3 Light::getPosition(HitRecord* rec, Vec3* normal, double* p)
{
	if(p) *p = 1;
	return pos_;
}

void Light::setMonteCarloMat()
{
	if (auto p = getGeometry()) {
		p->setMaterial(std::shared_ptr<Material>(new Material(getDiffuse(), Color(), Material::NORMAL)));
	}
}

FaceLight::FaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular)
	:Light(pos,ambient,diffuse,specular),right_(up^normal),up_(up),width_(width),height_(height),normal_(normal.normalize())
{
	setRandomEnabled(true);
	uoff_ = 0;
	voff_ = 0;
}

FaceLight::FaceLight(const FaceLight & light, rt::CopyOp copyop)
	:Light(light,copyop)
{
	uoff_	=light.uoff_;
	normal_ = light.normal_;
	voff_ = light.voff_;
	right_ = light.right_;
	up_ = light.up_;
	width_ = light.width_;
	height_ = light.height_;
}

//void FaceLight::setSamplePosition(double u, double v)
//{
//	uoff_ = u-0.5;
//	voff_ = v-0.5;
//}

Vec3 FaceLight::getPosition(HitRecord* rec, Vec3* normal, double* p)
{
	std::pair<double, double> uv;
	getRandomPair(&uv);
	Vec3 pos = Light::getPosition();
	pos += right_ * (uv.first-0.5) * width_ + up_ * (uv.second-0.5) * height_;
	if(p) *p = 1.0 / width_ / height_;
	if (normal) *normal = normal_;
	return pos;
}

MovableFaceLight::MovableFaceLight(Vec3 pos, Vec3 normal, Vec3 up, double width, double height, Color ambient, Color diffuse, Color specular)
	:FaceLight(pos,normal,up,width,height,ambient,diffuse,specular)
{
	radius_ = sqrt(pos.x_ * pos.x_ + pos.z_*pos.z_);
	orginPos_ = pos;
}

MovableFaceLight::MovableFaceLight(const MovableFaceLight & light, rt::CopyOp copyop)
	:FaceLight(light,copyop)
{
	radius_ = light.radius_;
	orginPos_ = light.orginPos_;
}

Vec3 MovableFaceLight::getPosition(HitRecord* rec, Vec3* normal, double* p)
{ 
	//绕z轴旋转
	pos_.x_ = sin(getTime()*2*3.1415926)*radius_;
	pos_.z_ = cos(getTime()*2*3.1415926)*radius_;
	return FaceLight::getPosition(rec,normal,p);
}

SphereLight::SphereLight(Vec3 pos, double radius, Color emission)
	:Light(pos,emission,emission,emission),radius_(radius)
{
	setRandomEnabled(true);
	sphere_.reset(new Sphere(pos, radius));
	std::shared_ptr<Material> mat(new Material(emission, Color(), Material::NORMAL));
	sphere_->setMaterial(mat);
}

SphereLight::SphereLight(const SphereLight & light, rt::CopyOp copyop)
	:Light(light,copyop),radius_(light.radius_)
{
	switch (copyop)
	{
	case rt::SHALLOW_COPY:
		break;
	case rt::RECUR_SHALLOW_COPY:
		sphere_ = light.sphere_->cloneToSharedPtr(copyop);
		break;
	case rt::DEEP_COPY:
		break;
	}
}

std::shared_ptr<Drawable> SphereLight::getGeometry()
{
	return sphere_;
}

Vec3 SphereLight::getPosition(HitRecord * rec, Vec3 * normal, double * p)
{
	std::pair<double, double> uv;
	getRandomPair(&uv);
	double ru = uv.first, rv = uv.second;
	Vec3 ppos = rec->ray.d * rec->t+ rec->ray.e;
	Vec3 dir = (pos_ - ppos);
	if (abs(dir.length() - radius_) < ZERO) { //如果是光源上本身的点，不考虑直射光
		if (p) *p = INFINITE*1.1;
		return Vec3(0, 0, 0);
	}
	//建立坐标系
	Vec3 w = dir.normalize(), u = ((fabs(w.x_) > 0.1 ? Vec3(0, 1, 0) : Vec3(1, 0, 0)) ^ w).normalize(), v = (w ^ u).normalize();
	double cosmax = sqrt(1 - radius_ * radius_ / (dir*dir));
	if (cosmax >= 1 - ZERO) {
		return Vec3(0, 0, 0);
	}
	//选取随机向量
	double cos_a = 1 - ru + ru * cosmax;
	double sin_a = sqrt(1 - cos_a * cos_a);
	double phi = 2 * PI * rv;
	Vec3 lightDir = u * cos(phi)*sin_a + v * sin(phi)*sin_a + w * cos_a;
	lightDir = lightDir.normalize();
	//转换为光源采样点坐标
	HitRecord temprec;
	sphere_->hit(Ray{ ppos,lightDir }, ZERO, INFINITE, &temprec);
	Vec3 hitp = ppos + lightDir * temprec.t; 
	Vec3 nl = (hitp - pos_).normalize();
	if (normal) *normal = nl;
	//计算概率
	if (p) {
		*p = nl*(-lightDir) / (2 * PI*(1 - cosmax)) / temprec.t / temprec.t;
		//if (*p < 0) {
		//	std::cout << *p << std::endl;
		//	std::cout << nl << std::endl;
		//	std::cout << lightDir << std::endl;
		//	std::cout << dir * dir << std::endl;
		//	std::cout << cosmax << std::endl;
		//	std::cout << cos_a << std::endl;
		//	std::cout << ppos << std::endl;
		//	std::cout << pos_ << std::endl;
		//	std::cout << nl * (-lightDir) << std::endl;
		//	std::cout << uv.first << std::endl;
		//}
	}
	return hitp;
}
