#pragma once
#include <vector>
#include <memory>
#include <set>
#include <map>
#include "singleton.h"
template<typename T>
using RTSmartPointer = std::shared_ptr<T>;

//class RandomPool : public Singleton<RandomPool>
//{
//	friend Singleton<RandomPool>;
//protected:
//	RandomPool();
//
//	//pairsnum = threadnum * ids * n * m  
//	std::vector<std::map<int, std::vector<std::pair<double, double>>>>  randomPairs_;
//};
namespace rt {
	enum CopyOp {
		SHALLOW_COPY = 0,		//新对象与源对象通过指针共享相同子对象
		RECUR_SHALLOW_COPY,		//为所有子对象也进行递归克隆，生成新的子对象，但是底层数据共享
		DEEP_COPY				//完全深层复制，子对象进行递归克隆，并且底层数据也进行复制
	};
};

#define META_Object(name) \
	RTObject* clone(const rt::CopyOp& copyop) const override { return new name (*this,copyop); } \
	std::shared_ptr<name> cloneToSharedPtr(const rt::CopyOp & copyop) const { return std::shared_ptr<name>(dynamic_cast<name*>(this->clone(copyop))); } \

class RTObject: public std::enable_shared_from_this<RTObject>
{
public:
	RTObject() {
		id_ = id++;
	}
	RTObject(const RTObject& object, rt::CopyOp copyop) {
		id_ = id++;
		setRandomSize(object.n_, object.m_);
		setRandomEnabled(object.randomIsAble_);
		setIteratorIndex( object.itIndex_);
		switch (copyop)
		{
		case rt::SHALLOW_COPY:
			break;
		case rt::RECUR_SHALLOW_COPY:
			break;
		case rt::DEEP_COPY:
			randomPairs_.insert(randomPairs_.begin(),object.randomPairs_.begin(), object.randomPairs_.end());
			break;
		default:
			break;
		}
		setTime(object.time_);
	}
	virtual ~RTObject();

	int getId() {
		return id_;
	}

	void addChild(RTObject* object);
	void addChild(RTSmartPointer<RTObject> object);

	virtual RTObject* clone(const rt::CopyOp& copyop) const = 0;

	//For Animation
	inline void setTime(double time);

	//For Random
	void setRandomSize(int n, int m = -1);

	void setRandomEnabled(bool able);

	void calRandom();

	int getRandomPair(int index, std::pair<double, double>* pair);
	int getRandomPair(std::pair<double, double>* pair);

	int setIteratorIndex(int index);

protected:
	inline double getTime() { return time_; }
	virtual void timeChanged(double time) {};

private:
	static int id;

	int id_;

	class cmp {
	public:
		bool operator()(const RTSmartPointer<RTObject>& lhs, const RTSmartPointer<RTObject>& rhs) {
			return lhs->getId() < rhs->getId();
		}
	};
	std::set<RTSmartPointer<RTObject>,cmp> children;

	//For Animation
	double time_=0;

	//For Random
	int n_=0;
	int m_=0;
	bool randomIsAble_=false;
	int itIndex_ = 0;
	std::vector<std::pair<double, double>> randomPairs_;
};

inline void RTObject::setTime(double time)
{
	timeChanged(time);
	for (auto& child : children) {
		child->setTime(time);
	}
	time_ = time;
}