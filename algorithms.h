#pragma once
#include <time.h>
#include <math.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <functional>
#include "types.h"
#include "RTObject.h"

namespace algorithm {
	inline void randFromDisk(double* x, double* y) {
		do {
			*x = (rand() * 1.0 / RAND_MAX - 0.5) * 2;
			*y = (rand() * 1.0 / RAND_MAX - 0.5) * 2;
		} while (*x * *x + *y * *y >= 1.0);
	}

	inline Vec3 genRandomUnitVec() {
		Vec3 a;
		do {
			a.x_ = rand()*1.0 / RAND_MAX * 2 - 1;
			a.y_ = rand()*1.0 / RAND_MAX * 2 - 1;
			a.z_ = rand()*1.0 / RAND_MAX * 2 - 1;
		} while (a.length() < 1);
		return a.normalize();
	}

	template<class T> void suffle(std::vector<T>& vec) {
		int n = vec.size();
		for (int i = n - 1; i >= 1; i--) {
			int slct = int(rand() * 1.0 / RAND_MAX * (i + 1));
			std::iter_swap(vec.begin() + slct, vec.begin() + i);
		}
	}

	//N指定柏林噪声常数
	inline double perlinNoise(double x, double y, double z, double scale, const std::vector<int>& P, const std::vector<Vec3>& G, int N) {
		assert(P.size() == N && G.size() == N);
		if (!(P.size() == N && G.size() == N)) return 0; //error

		x /= scale;
		y /= scale;
		z /= scale;
		double result = 0;
		for (int n = 0; n < 8; n++) { //遍历8个点
			int i = (n & 0x01) ? floor(x) : floor(x) + 1;
			int j = (n & 0x02) ? floor(y) : floor(y) + 1;
			int k = (n & 0x04) ? floor(z) : floor(z) + 1;
			double u = x - i, v = y - j, w = z - k;
			auto weight = [](double t) {
				double at = abs(t);
				if (at < 1) return 2 * at*at*at - 3 * at*at + 1;
				else return 0.0;
			};

			int rN = N;
			auto& rP = P;
			auto fai = [rN, rP](int t) {
				int m = t % rN;
				return rP[m >= 0 ? m : rN + m];
			};
			//int kds = P[(i + P[(j + P[k%N]) % N]) % N];
			//std::cout << P[(i + P[(j + P[k%N]) % N]) % N] << ' ' << (i + P[(j + P[k%N]) % N]) % N << ' ' << (j + P[k%N]) % N << ' ' << k % N << std::endl;
			result += weight(u)*weight(v)*weight(w)*(G[fai(i+fai(j+fai(k)))]*Vec3(u,v,w));
			
		}
		return result;         
	}

	class PerlinNoiseGenerator: public RTObject
	{
	public:
		PerlinNoiseGenerator(int N=256);
		PerlinNoiseGenerator(const PerlinNoiseGenerator& perlin, rt::CopyOp copyop);
		META_Object(PerlinNoiseGenerator)
		double getPerlinNoise(double x, double y, double z, double scale);
		double getPerlinNoiseLine(double x, double y, double z, double scale);
		double getPerlinNoiseNormalized(double x, double y, double z, double scale);
	protected:
		const unsigned int SEED = 12325; //随机种子
		int n_;
		static std::vector<int> P_;
		static std::vector<Vec3> G_;
	};

	class PerlinNoiseGeneratorWithTime: public PerlinNoiseGenerator
	{
	public:
		PerlinNoiseGeneratorWithTime(int N = 256);
		PerlinNoiseGeneratorWithTime(const PerlinNoiseGeneratorWithTime& perlin, rt::CopyOp copyop);
		META_Object(PerlinNoiseGeneratorWithTime)
		void timeChanged(double time);
	private:
		static std::vector<Vec3> randVec_s;
		static int count;
		static int stage;
		static double stime;
	};

	class IntervalArith { //区间数，用于隐函数曲面与直线求交
	public:
		IntervalArith() : a_(0), b_(0) {}
		IntervalArith(double a, double b)
			:a_(a), b_(b) {}
		IntervalArith(double a)
			:a_(a), b_(a) {}
		IntervalArith operator+(const IntervalArith& rhs) { return IntervalArith(a_ + rhs.a_, b_ + rhs.b_); }
		IntervalArith operator-(const IntervalArith& rhs) { return IntervalArith(a_ - rhs.b_, b_ - rhs.a_); }
		IntervalArith operator-() { return IntervalArith()-*this; }
		IntervalArith operator*(const IntervalArith& rhs) { return IntervalArith(std::min({ a_*rhs.a_, a_*rhs.b_, b_*rhs.a_, b_*rhs.b_ }), std::max({ a_*rhs.a_, a_*rhs.b_, b_*rhs.a_, b_*rhs.b_ }));}
		IntervalArith operator*(const double& rhs) { return IntervalArith(a_*rhs,b_*rhs); }
		IntervalArith operator/(const IntervalArith& rhs) { return IntervalArith(std::min({ a_/rhs.a_, a_/rhs.b_, b_/rhs.a_, b_/rhs.b_ }), std::max({ a_/rhs.a_, a_/rhs.b_, b_/rhs.a_, b_/rhs.b_ }));}
		void getInterval(int& a, int& b) { a = a_; b = b_; }
		bool includeZero() { return a_ * b_ <= 0.0; }
		double a() { return a_; }
	private:
		double a_, b_;
	};

	typedef std::function<IntervalArith(IntervalArith)> IntervalFunc;
	typedef std::function<double(double)> DoubleFunc;
	const double PRECISE = 10e-5;

	inline int calSingleRoot(IntervalFunc ifunc, double t1, double t2, double* root) //二分法求根
	{
		auto func = [ifunc](double d) {
			return ifunc(d).a();
		};
		double f1 = func(t1), f2 = func(t2);
		if (f1 == 0.0) {
			*root = t1;
			return 0;
		}
		if (f2 == 0.0) {
			*root = t2;
			return 0;
		}
		if (f1*f2 > 0.0)
			return -1;
		double tm = (t1 + t2) / 2;
		double fm = func(tm);
		while (abs(fm) > PRECISE) {
			if (f1*fm < 0) {
				t2 = tm;
				f2 = fm;
			}
			else {
				t1 = tm;
				f1 = fm;
			}
			tm = (t1 + t2) / 2;
			fm = func(tm);
		}
		*root = tm;
		return 0;
	}

	//求解函数在给定范围内的最小根, 输入：函数、导函数、下界、上界
	//无根返回-1
	inline int calMinRoots(IntervalFunc func, IntervalFunc funcd, double t0, double t1, double* root) { 
		if (t0 > t1) return -1;
		IntervalArith inter = func(IntervalArith(t0, t1));
		if (!inter.includeZero()) return -1;
		if (abs(t1 - t0) <= PRECISE) {
			*root = t1;
			return 0;
		}
		if (!funcd(IntervalArith(t0, t1)).includeZero()) {
			if (calSingleRoot(func, t0, t1, root))
				return -1;
			else
				return 0;
		}
		else {
			double tm = (t0+t1)/2;
			if (calMinRoots(func, funcd, t0, tm, root) == 0)
				return 0;
			if(calMinRoots(func, funcd, tm, t1, root)==0)
				return 0;
		}
		return  -1;
	}
}