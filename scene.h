#pragma once
#include <memory>
#include <vector>
#include "Surface.h"
#include "light.h"
#include "types.h"
#include "camera.h"
class Scene: public RTObject
{
public:
	META_Object(Scene)

	Scene();
	~Scene();
	Scene(const Scene& scene, rt::CopyOp copyop);

	//指定像素大小
	int setPixels(int x, int y);
	int getPixels(int* x, int* y);

	void setRootNode(std::shared_ptr<Node> node);
	void setCamera(std::shared_ptr<Camera> camera);
	void setClearColor(Color color);
	void addLight(std::shared_ptr<Light> light);

	void run(int x=-1,int y=-1);
	void runParallelly(int threadNum);
	inline bool isPixelsFinished(int x, int y);
	//query color in the indicated pixel. Must be called after called run(), or return -1;
	inline int queryColor(unsigned int x,unsigned int y,Color& color);
	//Must be called after called run()
	int saveToBpm(const char* filaname);

	Color rayColor(Ray ray, double t0, double t1, int jumptime = 0);
	//__global__ void rayColor(Ray ray, double t0, double t1, int jumptime, Color& cout);

private:
	inline void setPixelColor(int x, int y, Color color);

	std::shared_ptr<Node> node_;
	std::shared_ptr<Camera> camera_;
	std::vector<std::shared_ptr<Light>> lights;
	Color backGround_;
	int px_;
	int py_;

	bool run_;
	std::vector<std::vector<Color>> colorArray_;

	//int sampleNum_=15;
	int sampleNum_ = 6;
	int maxJump_=10;

};
void getRayColor(Scene* scene, std::vector<Ray> ray, double t0, double t1, int jumptime, std::vector<Color>& color);


inline void Scene::setPixelColor(int x, int y, Color color)
{
	colorArray_[x][y] = color;
}

inline bool Scene::isPixelsFinished(int x, int y)
{
	if (colorArray_[x][y] == Color(-1, -1, -1, -1))
		return false;
	else true;
}

inline int Scene::queryColor(unsigned int x, unsigned int y, Color& color)
{
	if (run_) {
		if (x >= px_ || y >= py_) return -1;
		color = colorArray_[x][y];
		return 0;
	}
	else return -1;
}