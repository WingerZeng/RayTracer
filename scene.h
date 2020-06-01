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
	Scene();
	~Scene();

	//指定像素大小
	int setPixels(int x, int y);
	int getPixels(int* x, int* y);

	void setRootNode(std::shared_ptr<Node> node);
	void setCamera(std::shared_ptr<Camera> camera);
	void setClearColor(Color color);
	void addLight(std::shared_ptr<Light> light);
	
	void run(int x=-1,int y=-1);
	//query color in the indicated pixel. Must be called after called run(), or return -1;
	int queryColor(unsigned int x,unsigned int y,Color& color);
	//Must be called after called run()
	int saveToBpm(const char* filaname);

	Color rayColor(Ray ray, double t0, double t1, int jumptime = 0);
	//__global__ void rayColor(Ray ray, double t0, double t1, int jumptime, Color& cout);

private:
	std::shared_ptr<Node> node_;
	std::shared_ptr<Camera> camera_;
	std::vector<std::shared_ptr<Light>> lights;
	Color backGround_;
	int px_;
	int py_;

	bool run_;
	std::vector<std::vector<Color>> colorArray_;

	int sampleNum_=25;
	int maxJump_=8;
};
void getRayColor(Scene* scene, std::vector<Ray> ray, double t0, double t1, int jumptime, std::vector<Color>& color);