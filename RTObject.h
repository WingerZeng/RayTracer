#pragma once
#include <vector>
#include <memory>
#include <set>

template<typename T>
using RTSmartPointer = std::shared_ptr<T>;

class RTObject: public std::enable_shared_from_this<RTObject>
{
public:
	RTObject() {
		id_ = id++;
	}
	virtual ~RTObject();

	int getId() {
		return id_;
	}

	void addChild(RTObject* object);
	void addChild(RTSmartPointer<RTObject> object);

	//For Random
	void setRandomSize(int n, int m = -1);

	void setRandomEnabled(bool able);

	void calRandom();

	int getRandomPair(int index, std::pair<double, double>* pair);
	int getRandomPair(std::pair<double, double>* pair);

	int setIteratorIndex(int index);
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
	//For Random
	int n_=0;
	int m_=0;
	bool randomIsAble_=false;
	int itIndex_ = 0;
	std::vector<std::pair<double, double>> randomPairs_;
};