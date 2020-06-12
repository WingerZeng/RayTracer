#pragma once
#include "types.h"
#include "RTObject.h"
#include "definitions.h"
#include "algorithms.h"
#include "definitions.h"
class Material : public RTObject
{
public:
	Material(Color amb, Color dif, Color spe, double shi); //NORMAL MERERIAL
	Material(Color amb, Color dif, Color spe, Color mir, double shi); //SPECULAR MERERIAL
	Material(Color a, double nr); //TRANSPARENT MERERIAL
	Material(const Material& mat, rt::CopyOp copyop);
	Material() = default;

	META_Object(Material);

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
	virtual void setPosition(Vec3 pos) {};

	virtual Type getType() { return type; }
	virtual Color getAmbient() { return ambient; }
	virtual Color getDiffuse() { return diffuse; }
	virtual Color getSpecular() { return specular; }
	virtual Color getMirror() { return mirror; }
	virtual double getShine() { return shine; }
	virtual double getMirrorBlur() { return blur; }
	virtual double getRefraCoef() { return nr_; }
	virtual Color getAttenuation() { return a_; }
	virtual Vec3 getNormal(Vec3 Normal) { return Normal; }; //return normal after adjust

private:
	Type type;
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
	Color getAmbient() override { return Material::getAmbient() & Color(noise_, noise_, noise_, 1); }
	Color getDiffuse() override { return Material::getDiffuse() & Color(noise_, noise_, noise_, 1); }
	Color getSpecular() override { return Material::getSpecular() & Color(noise_, noise_, noise_, 1); }
	void setPosition(Vec3 pos) override { noise_ = perlin_->getPerlinNoise(pos.x_, pos.y_, pos.z_, scale_); pos_ = pos; }

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
	Color getAmbient() override {
		using std::max;
		using std::min;
		auto hf1 = min(1.0, max(0.0, max(linenoise_ - 0.6, 0.6 - linenoise_)*2.5 - 0.05));
		auto hf2 = min(1.0, max(0.0, max(noise_ - 0.4, 0.4 - noise_) * 5 - 0.05));
		auto hf = hf1 * hf2;
		double linenoise_ = 1 - this->linenoise_;
		double lineHigh = (linenoise_)*1.7;
		double noise_ = this->noise_ / 2.5;
		return (((Material::getAmbient() & Color(1 - noise_, 1 - noise_, 1 - noise_, 1)) + (Color(0.45, 0, 0, 1) & Color(lineHigh + noise_, lineHigh + noise_, lineHigh + noise_, 1)))&Color(hf, hf, hf, 1))*0.35;
	}
	Color getDiffuse() override {
		using std::max;
		using std::min;
		auto hf1 = min(1.0, max(0.0, max(linenoise_ - 0.6, 0.6 - linenoise_)*2.5 - 0.05));
		auto hf2 = min(1.0, max(0.0, max(noise_ - 0.4, 0.4 - noise_) * 5 - 0.05));
		auto hf = hf1 * hf2;
		double linenoise_ = 1 - this->linenoise_;
		double lineHigh = (linenoise_)*1.7;
		double noise_ = this->noise_ / 2.5;
		return (((Material::getDiffuse() & Color(1 - noise_, 1 - noise_, 1 - noise_, 1)) + (Color(0.45, 0, 0, 1) & Color(lineHigh + noise_, lineHigh + noise_, lineHigh + noise_, 1)))&Color(hf, hf, hf, 1))*1.0;
	}
	Color getSpecular() override {
		using std::max;
		using std::min;
		auto hf = min(1.0, max(0.0, max(linenoise_ - 0.25, 0.25 - linenoise_)*4.5 - 0.05));
		return (Material::getSpecular() & Color(noise_, noise_, noise_, 1)) + (Color(0.55, 0, 0, 1) & Color(1 - noise_, 1 - noise_, 1 - noise_, 1))&Color(hf, hf, hf, 1) * 0.3;
	}
	void setPosition(Vec3 pos) override;
	Vec3 getNormal(Vec3 Normal) override; //管的中间突出来
private:
	double linenoise_ = 1;
	std::shared_ptr<algorithm::PerlinNoiseGeneratorWithTime> lineperlin_;
};