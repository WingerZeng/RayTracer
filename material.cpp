#include "material.h"
Material::Material(Color amb, Color dif, Color spe, double shi)
	:ambient(amb), diffuse(dif), specular(spe), shine(shi)
{
	type = NORMAL;
}

Material::Material(Color amb, Color dif, Color spe, Color mir, double shi)
	: ambient(amb), diffuse(dif), specular(spe), mirror(mir), shine(shi)
{
	type = Type(SPECULAR | NORMAL);
}

Material::Material(Color emission, Color color, Material::Type tp)
	:type(Type(tp|MONTECARLO)),emission(emission),diffuse(color)
{
}

Material::Material(Color a, double nr)
	:a_(a),nr_(nr)
{
	type = TRANSPARENT;
}

Material::Material(const Material & mat, rt::CopyOp copyop)
	:RTObject(mat,copyop)
{
	type = mat.type;
	emission = mat.emission;
	ambient = mat.ambient;
	diffuse = mat.diffuse;
	specular = mat.specular;
	mirror = mat.mirror;
	shine = mat.shine;
	blur = mat.blur;
	a_ = mat.a_; 
	nr_ = mat.nr_;
}

void Material::setMonteCarlo(Color color, Color emission)
{
	type = Type(type | MONTECARLO);
	if(color.x_ >= 0)
		diffuse = color;
	if(emission.x_ >= 0)
		this->emission = emission;
}

PerlinNoiseNormalMaterial_Blood::PerlinNoiseNormalMaterial_Blood(const PerlinNoiseNormalMaterial_Blood & mat, rt::CopyOp copyop)
	:PerlinNoiseNormalMaterial(mat,copyop)
{
	switch (copyop)
	{
	case rt::SHALLOW_COPY:
		break;
	case rt::RECUR_SHALLOW_COPY:
		lineperlin_ = mat.lineperlin_->cloneToSharedPtr(copyop);
		break;
	case rt::DEEP_COPY:
		break;
	default:
		break;
	}
	addChild(lineperlin_);
}

void PerlinNoiseNormalMaterial_Blood::setPosition(Vec3 pos) {
	noise_ = std::min(perlin_->getPerlinNoiseNormalized(pos.x_, pos.y_, pos.z_, scale_)*1.5, 1.0);
	linenoise_ = lineperlin_->getPerlinNoiseLine(pos.x_, pos.y_, pos.z_, scale_);
	pos_ = pos;
}

Vec3 PerlinNoiseNormalMaterial_Blood::getNormal(Vec3 normal)
{
	Vec3 vec(0, 0, 1);
	if ((normal^vec).isZero()) {
		vec = Vec3(1, 0, 0);
	}
	Vec3 b = (normal ^ vec).normalize();
	Vec3 t = (b ^ normal).normalize();
	Vec3 pos = pos_ + t * 0.001;
	double dPerlin_b = linenoise_ - lineperlin_->getPerlinNoiseLine(pos.x_, pos.y_, pos.z_, scale_);
	pos = pos_ + b * 0.001;
	double dPerlin_t = linenoise_ - lineperlin_->getPerlinNoiseLine(pos.x_, pos.y_, pos.z_, scale_);
	return (normal + b * dPerlin_b * 60 + t * dPerlin_t * 60).normalize();
}

PerlinNoiseNormalMaterial::PerlinNoiseNormalMaterial(const PerlinNoiseNormalMaterial & mat, rt::CopyOp copyop)
	:Material(mat,copyop)
{
	scale_ = mat.scale_;
	switch (copyop)
	{
	case rt::SHALLOW_COPY:
		break;
	case rt::RECUR_SHALLOW_COPY:
		perlin_ = mat.perlin_->cloneToSharedPtr(copyop);
		break;
	case rt::DEEP_COPY:
		break;
	default:
		break;
	}
	addChild(perlin_);
}
