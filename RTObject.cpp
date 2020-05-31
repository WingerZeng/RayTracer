#include "RTObject.h"
#include <time.h>
#include <algorithm>
#include <assert.h>
#include <iostream>
RTObject::~RTObject()
{
}

void RTObject::addChild(RTObject * object)
{
	addChild(object->shared_from_this());
}

void RTObject::addChild(RTSmartPointer<RTObject> object)
{
	children.insert(object);
	object->setRandomSize(n_, m_);
	object->setIteratorIndex(itIndex_);
}


//For Random

inline void RTObject::setRandomSize(int n, int m) {
	for (const auto& child : children) {
		child->setRandomSize(n, m);
	}

	if (n == -1) {
		setRandomEnabled(false);
		return;
	}
	if (m == -1) {
		m_ = n_ = n;
	}
	else {
		n_ = n;
		m_ = m;
	}
}

void RTObject::setRandomEnabled(bool able)
{
	randomIsAble_ = able;
}

void RTObject::calRandom()
{
	//jitter sample
	if (!randomIsAble_) return;
	randomPairs_.clear();
	srand(time(0));
	double nstep = 1.0 / n_;
	double mstep = 1.0 / m_;
	for (int i = 0; i < m_; i++) {
		for (int j = 0; j < n_; j++) {
			double nrand = nstep * rand() / RAND_MAX;
			double mrand = mstep * rand() / RAND_MAX;
			randomPairs_.push_back(std::make_pair(j*nstep+nrand,i*mstep+mrand));
		}
	}
	//shuffle
	for (int i = randomPairs_.size() - 1; i >= 1;i--) {
		int randIndex = (rand() / RAND_MAX) * (i+1);
		std::iter_swap(randomPairs_.begin() + randIndex, randomPairs_.begin() + i);
	}
	for (const auto& child : children) {
		child->calRandom();
	}
}

int RTObject::getRandomPair(int index, std::pair<double, double>* pair)
{
	if (!randomIsAble_) return -1;
	if (index < randomPairs_.size()) {
		*pair = randomPairs_[index];
		return 0;
	}
	assert(0);
	return -1;
}

int RTObject::getRandomPair(std::pair<double, double>* pair)
{
	getRandomPair(itIndex_, pair);
	return 0;
}

int RTObject::setIteratorIndex(int index)
{
	if (index >= n_ * m_) return -1;
	itIndex_ = index;
	for (const auto& child : children) {
		child->setIteratorIndex(index);
	}
	return 0;
}

int RTObject::id = 0;