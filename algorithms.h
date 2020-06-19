#pragma once
#include <time.h>
#include <math.h>
#include <assert.h>
#include <vector>
#include <array>
#include <algorithm>
#include <functional>
#include "types.h"
#include "RTObject.h"

namespace algorithm {
	inline Vec3 getBoundPoint(int index, const BoundBox_t bound);

	inline Vec3 firstIntersection2Sphere(Ray ray, const Vec3& center, double radius, double& length) {
		Vec3 dir = (center - ray.e);
		double dist = dir.length();
		double cos = ray.d * dir.normalize();
		double coslen = dist * cos;
		double sinlen = dist * sqrt(1 - cos * cos);
		double dlen = sqrt(radius*radius - sinlen * sinlen);
		length = coslen - dlen;
		return ray.e + ray.d * length;
	}

	inline double rand01() {
		return rand()*1.0 / RAND_MAX;
	}

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
			auto fai = [&rN, &rP](int t) {
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
	const double PRECISE = 10e-8;

	inline int calSingleRoot(DoubleFunc ifunc, double t1, double t2, double* root) //二分法求根
	{
		double f1 = ifunc(t1), f2 = ifunc(t2);
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
		double fm = ifunc(tm);
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
			fm = ifunc(tm);
		}
		*root = tm;
		return 0;
	}

	inline int calSingleRoot(IntervalFunc ifunc, double t1, double t2, double* root) //二分法求根
	{
		auto func = [ifunc](double d) {
			return ifunc(d).a();
		};
		return calSingleRoot(func, t1, t2, root);
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

	inline bool hitBox(Vec3 bound[2], Ray ray, double t0, double t1, HitRecord* rec) {
		Vec3 vt0 = (bound[0] - ray.e) / ray.d;
		Vec3 vt1 = (bound[1] - ray.e) / ray.d;
		double tmin[4]{ std::min(vt0.x_,vt1.x_),std::min(vt0.y_,vt1.y_),std::min(vt0.z_,vt1.z_),t0 };
		double tmax[4]{ std::max(vt0.x_,vt1.x_),std::max(vt0.y_,vt1.y_),std::max(vt0.z_,vt1.z_),t1 };
		double r0 = *std::max_element(tmin, tmin + 4);
		double r1 = *std::min_element(tmax, tmax + 4);
		if (r0 <= r1) {
			if (rec) rec->t = r0;
			return true;
		}
		return false;
	}

	//返回射线击中包围盒的范围
	inline bool hitBox(const BoundBox_t bound, Ray ray, double t0, double t1, double* rt0, double* rt1) {
		Vec3 vt0 = (bound[0] - ray.e) / ray.d;
		Vec3 vt1 = (bound[1] - ray.e) / ray.d;
		double tmin[4]{ std::min(vt0.x_,vt1.x_),std::min(vt0.y_,vt1.y_),std::min(vt0.z_,vt1.z_),t0 };
		double tmax[4]{ std::max(vt0.x_,vt1.x_),std::max(vt0.y_,vt1.y_),std::max(vt0.z_,vt1.z_),t1 };
		double r0 = *std::max_element(tmin, tmin + 4);
		double r1 = *std::min_element(tmax, tmax + 4);
		if (r0<=r1) {
			*rt0 = r0;
			*rt1 = r1;
			return true;
		}
		return false;
	}

	enum BoxFace {
		FRONT = 0,   //On xOz plane. Normal is -y; 
		TOP,  
		LEFT,
		RIGHT,
		DOWN, //On xOy plane. Normal is -z;
		BACK,
	};

	inline BoxFace getBoundOppoFace(BoxFace face) {
		return BoxFace(5 - face);
	}

	inline void getAdjBox(BoundBox_t box, BoxFace face, BoundBox_t adjbox) {
		switch (face)
		{
		case algorithm::FRONT:
			adjbox[0] = box[0] - Vec3(0, box[1].y_ - box[0].y_, 0);
			adjbox[1] = getBoundPoint(5, box);
			break;
		case algorithm::TOP:
			adjbox[0] = getBoundPoint(4, box);
			adjbox[1] = box[1] + Vec3(0, 0, box[1].z_ - box[0].z_);
			break;
		case algorithm::LEFT:
			adjbox[0] = box[0] - Vec3(box[1].x_ - box[0].x_, 0, 0);
			adjbox[1] = getBoundPoint(7, box);
			break;
		case algorithm::RIGHT:
			adjbox[0] = getBoundPoint(1, box);
			adjbox[1] = box[1] + Vec3(box[1].x_ - box[0].x_, 0, 0);
			break;
		case algorithm::DOWN:
			adjbox[0] = box[0] - Vec3(0, 0, box[1].z_ - box[0].z_);
			adjbox[1] = getBoundPoint(2, box);
			break;
		case algorithm::BACK:
			adjbox[0] = getBoundPoint(3, box);
			adjbox[1] = box[1] + Vec3(0, box[1].y_ - box[0].y_, 0 );
			break;
		}
	}

	inline std::array<int,4> getBoundFacePoint(BoxFace face) {
		switch (face)
		{
		case algorithm::FRONT:
			return { 0,1,5,4 };
		case algorithm::BACK:
			return { 3,2,6,7 };
		case algorithm::TOP:
			return { 4,5,6,7 };
		case algorithm::DOWN:
			return { 0,1,2,3 };
		case algorithm::LEFT:
			return { 3,0,4,7 };
		case algorithm::RIGHT:
			return { 1,2,6,5 };
		}
	}

	inline Vec3 getBoundPoint(int index, const BoundBox_t bound) {
		int ix;
		int iy;
		switch (index % 4)
		{
		case 0:
			ix = iy = 0;
			break;
		case 1:
			ix = 1;
			iy = 0;
			break;
		case 2:
			ix = iy = 1;
			break;
		case 3:
			ix = 0;
			iy = 1;
			break;
		}
		int iz = (index >> 2) & 1;
		return Vec3(bound[ix].x_, bound[iy].y_, bound[iz].z_);
	}

	inline bool isPointInsideBoundBox(BoundBox_t bound, Point_t point) {
		return (point.x_ >= bound[0].x_&&point.x_ <= bound[1].x_) &&
				(point.y_ >= bound[0].y_&&point.y_ <= bound[1].y_) &&
				(point.z_ >= bound[0].z_&&point.z_ <= bound[1].z_);
	}

	inline bool isBoundBoxOverLap(BoundBox_t bound1, BoundBox_t bound2) {
		return (std::max(bound1[0].x_, bound2[0].x_) < std::min(bound1[1].x_, bound2[1].x_)) &&
			(std::max(bound1[0].y_, bound2[0].y_) < std::min(bound1[1].y_, bound2[1].y_)) &&
			(std::max(bound1[0].z_, bound2[0].z_) < std::min(bound1[1].z_, bound2[1].z_));
	}

	inline bool isBoundInsideBound(BoundBox_t  outerbound, BoundBox_t innerbound) {
		return isPointInsideBoundBox(outerbound, innerbound[0]) && isPointInsideBoundBox(outerbound, innerbound[1]);
	}
}