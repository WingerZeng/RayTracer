#pragma once
template<class T>
class Singleton
{
protected:
	Singleton();
	virtual ~Singleton();
	T* getSingletonPtr();

	static T* sig;
};

template<class T> T* Singleton<T>::sig = nullptr;

template<class T>
Singleton<T>::Singleton()
{
}


template<class T>
Singleton<T>::~Singleton()
{
}

template<class T>
T* Singleton<T>::getSingletonPtr()
{
	if (!sig) sig = new T;
	return sig;
}
