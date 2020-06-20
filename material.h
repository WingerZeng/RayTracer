#pragma once
#include "types.h"
#include "RTObject.h"
#include "definitions.h"
#include "algorithms.h"
#include "definitions.h"
class Material : public RTObject
{
public:
	enum Type {
		NULLTYPE = 0x0,
		NORMAL = 0x1,
		SPECULAR = 0x2,
		TRANSPARENT = 0x4,
		MONTECARLO = 0x8
	};

	Material(Color amb, Color dif, Color spe, double shi); //NORMAL MERERIAL
	Material(Color amb, Color dif, Color spe, Color mir, double shi); //SPECULAR MERERIAL
	//For Monte Carlo Material, type can be NORMAL（Diffuse), SPECULAR, TRANSPARENT
	Material(Color emission, Color color, Material::Type type); 
	Material(Color a, double nr); //TRANSPARENT MERERIAL
	Material(const Material& mat, rt::CopyOp copyop);
	Material() = default;

	META_Object(Material);

	virtual void setMonteCarlo(Color color = Color(-1, -1, -1, 0), Color emission = Color(-1, -1, -1, 0));
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
	virtual void setPosition(Vec3 pos) {};

	virtual Type getType() { return type; }
	virtual bool isMonteCarlo() { return type & MONTECARLO; }
	virtual Color getColor(HitRecord* rec = nullptr) { return diffuse; } //for Monte Carlo Path Tracing
	virtual Color getEmission(HitRecord* rec = nullptr) { return emission; } //for Monte Carlo Path Tracing
	virtual Color getAmbient(HitRecord* rec = nullptr) { return ambient; }
	virtual Color getDiffuse(HitRecord* rec = nullptr) { return diffuse; }
	virtual Color getSpecular(HitRecord* rec = nullptr) { return specular; }
	virtual Color getMirror() { return mirror; }
	virtual double getShine() { return shine; }
	virtual double getMirrorBlur() { return blur; }
	virtual double getRefraCoef() { return nr_; }
	virtual Color getAttenuation() { return a_; }
	virtual Vec3 getNormal(Vec3 Normal) { return Normal; }; //return normal after adjust

private:
	Type type;
	Color emission;
	Color ambient;
	Color diffuse;
	Color specular;
	Color mirror;
	double shine;
	double blur = 0;
	//for refraction
	Color a_; //光强损失
	double nr_; //refraction coefficient
};

//For Normal Check
class NormalCheckMaterial :public Material {
public:
	NormalCheckMaterial()
		:Material(Color(0, 0, 0, 1), Color(0, 0, 0, 1), Color(0, 0, 0, 1), 1) {}
	NormalCheckMaterial(const NormalCheckMaterial& mat, rt::CopyOp copyop)
		:Material(mat, copyop) {}
	META_Object(NormalCheckMaterial)
	//Color getAmbient(HitRecord* rec = nullptr) override { return Color((rec->normal.normalize() + Vec3(1,1,1))/2, 1.0); } //区分正负
	Color getAmbient(HitRecord* rec = nullptr) override { return Color( rec->normal.calabs(), 1.0); }
};

class PositionCheckMaterial :public Material {
public:
	PositionCheckMaterial(BoundBox_t box)
		:Material(Color(0, 0, 0, 1), Color(0, 0, 0, 1), Color(0, 0, 0, 1), 1), box_{box[0],box[1]} {}
	PositionCheckMaterial(const PositionCheckMaterial& mat, rt::CopyOp copyop)
		:Material(mat, copyop), box_{mat.box_[0],mat.box_[1]} {}
	META_Object(PositionCheckMaterial)
	Color getAmbient(HitRecord* rec = nullptr) override { 
		return Color((rec->localp-box_[0])/(box_[1]-box_[0]), 1.0); }
private:
	BoundBox_t box_;
};


class PerlinNoiseNormalMaterial : public Material
{
public:
	PerlinNoiseNormalMaterial(Color amb, Color dif, Color spe, double shi, double scale = 1)
		:Material(amb, dif, spe, shi), scale_(scale) {
		perlin_.reset(new algorithm::PerlinNoiseGeneratorWithTime);
		addChild(perlin_);
	}
	PerlinNoiseNormalMaterial(const PerlinNoiseNormalMaterial& mat, rt::CopyOp copyop);
	META_Object(PerlinNoiseNormalMaterial)
	Color getAmbient(HitRecord* rec = nullptr) override { return Material::getAmbient(rec) & Color(noise_, noise_, noise_, 1); }
	Color getDiffuse(HitRecord* rec = nullptr) override { return Material::getDiffuse(rec) & Color(noise_, noise_, noise_, 1); }
	Color getSpecular(HitRecord* rec = nullptr) override { return Material::getSpecular(rec) & Color(noise_, noise_, noise_, 1); }
	void setPosition(Vec3 pos) override { noise_ = perlin_->getPerlinNoiseNormalized(pos.x_, pos.y_, pos.z_, scale_); pos_ = pos; }

protected:
	std::shared_ptr<algorithm::PerlinNoiseGeneratorWithTime> perlin_;
	double noise_ = 1;
	double scale_;
	Vec3 pos_;
};

class PerlinNoiseNormalMaterial_Blood : public PerlinNoiseNormalMaterial
{
public:
	PerlinNoiseNormalMaterial_Blood(Color amb, Color dif, Color spe, double shi, double scale = 1)
		:PerlinNoiseNormalMaterial(amb, dif, spe, shi, scale) {
		lineperlin_.reset(new algorithm::PerlinNoiseGeneratorWithTime);
		addChild(lineperlin_);
	}
	PerlinNoiseNormalMaterial_Blood(const PerlinNoiseNormalMaterial_Blood& mat, rt::CopyOp copyop);
	META_Object(PerlinNoiseNormalMaterial_Blood)
	using PerlinNoiseNormalMaterial::PerlinNoiseNormalMaterial;
	Color getAmbient(HitRecord* rec = nullptr) override {
		using std::max;
		using std::min;
		auto hf1 = min(1.0, max(0.0, max(linenoise_ - 0.6, 0.6 - linenoise_)*2.5 - 0.05));
		auto hf2 = min(1.0, max(0.0, max(noise_ - 0.4, 0.4 - noise_) * 5 - 0.05));
		auto hf = hf1 * hf2;
		double linenoise_ = 1 - this->linenoise_;
		double lineHigh = (linenoise_)*1.7;
		double noise_ = this->noise_ / 2.5;
		return (((Material::getAmbient(rec) & Color(1 - noise_, 1 - noise_, 1 - noise_, 1)) + (Color(0.45, 0, 0, 1) & Color(lineHigh + noise_, lineHigh + noise_, lineHigh + noise_, 1)))&Color(hf, hf, hf, 1))*0.35;
	}
	Color getDiffuse(HitRecord* rec = nullptr) override {
		using std::max;
		using std::min;
		auto hf1 = min(1.0, max(0.0, max(linenoise_ - 0.6, 0.6 - linenoise_)*2.5 - 0.05));
		auto hf2 = min(1.0, max(0.0, max(noise_ - 0.4, 0.4 - noise_) * 5 - 0.05));
		auto hf = hf1 * hf2;
		double linenoise_ = 1 - this->linenoise_;
		double lineHigh = (linenoise_)*1.7;
		double noise_ = this->noise_ / 2.5;
		return (((Material::getDiffuse(rec) & Color(1 - noise_, 1 - noise_, 1 - noise_, 1)) + (Color(0.45, 0, 0, 1) & Color(lineHigh + noise_, lineHigh + noise_, lineHigh + noise_, 1)))&Color(hf, hf, hf, 1))*1.0;
	}
	Color getSpecular(HitRecord* rec = nullptr) override {
		using std::max;
		using std::min;
		auto hf1 = min(1.0, max(0.0, max(linenoise_ - 0.6, 0.6 - linenoise_)*2.5 - 0.05));
		auto hf2 = min(1.0, max(0.0, max(noise_ - 0.4, 0.4 - noise_) * 5 - 0.05));
		auto hf = hf1 * hf2;
		return (Material::getSpecular(rec) & Color(noise_, noise_, noise_, 1)) + (CenterColor & Color(1 - noise_, 1 - noise_, 1 - noise_, 1))&Color(hf, hf, hf, 1) * 0.85;
	}
	void setPosition(Vec3 pos) override;
	Vec3 getNormal(Vec3 Normal) override; //管的中间突出来
private:
	const Color CenterColor = Color(0.45, 0, 0, 1);
	double linenoise_ = 1;
	std::shared_ptr<algorithm::PerlinNoiseGeneratorWithTime> lineperlin_;
};