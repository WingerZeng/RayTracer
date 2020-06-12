#include "algorithms.h"
std::vector<int> algorithm::PerlinNoiseGenerator::P_;
std::vector<Vec3> algorithm::PerlinNoiseGenerator::G_;
algorithm::PerlinNoiseGenerator::PerlinNoiseGenerator(int N)
	:n_(N)
{
	srand(SEED);
	if (P_.empty()) {
		//预先生成随机单位向量
		for (int i = 0; i < N; i++) {
			G_.push_back(genRandomUnitVec());
		}
		//预先生成自然数排列
		for (int i = 0; i < N; i++) {
			P_.push_back(i);
		}
		suffle(P_);
	}
}

algorithm::PerlinNoiseGenerator::PerlinNoiseGenerator(const PerlinNoiseGenerator & perlin, rt::CopyOp copyop)
	:RTObject(perlin,copyop)
{
	n_ = perlin.n_;
}

double algorithm::PerlinNoiseGenerator::getPerlinNoise(double x, double y, double z, double scale)
{
	return perlinNoise(x, y, z, scale, P_, G_, n_);
}

double algorithm::PerlinNoiseGenerator::getPerlinNoiseLine(double x, double y, double z, double scale)
{
	return std::min(abs(perlinNoise(x, y, z, scale, P_, G_, n_)) * 11, 1.0);
}

double algorithm::PerlinNoiseGenerator::getPerlinNoiseNormalized(double x, double y, double z, double scale)
{
	return perlinNoise(x, y, z, scale, P_, G_, n_) / 2 + 0.5;
}

int algorithm::PerlinNoiseGeneratorWithTime::count = 0;
int algorithm::PerlinNoiseGeneratorWithTime::stage = 0;
std::vector<Vec3> algorithm::PerlinNoiseGeneratorWithTime::randVec_s;
double algorithm::PerlinNoiseGeneratorWithTime::stime=0;

algorithm::PerlinNoiseGeneratorWithTime::PerlinNoiseGeneratorWithTime(int N)
	:PerlinNoiseGenerator(N)
{
}

algorithm::PerlinNoiseGeneratorWithTime::PerlinNoiseGeneratorWithTime(const PerlinNoiseGeneratorWithTime & perlin, rt::CopyOp copyop)
	:PerlinNoiseGenerator(perlin,copyop)
{
}

void algorithm::PerlinNoiseGeneratorWithTime::timeChanged(double time)
{
	if (time == stime) return;
	else {
		double dt = time - getTime();
		if (count % 6 == 0) { //每20下改变一个新的方向
			randVec_s.clear();
			for (int i = 0; i < n_; i++) {
				randVec_s.push_back(genRandomUnitVec());
			}
			count = 1;
			stage++;
		}
		else count++;
		for (int i = 0; i < G_.size(); i++) {
			G_[i] = G_[i] + G_[(i+stage)%n_] * dt * 5;
			G_[i] = G_[i].normalize();
		}
		stime = time;
	}
}
