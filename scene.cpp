#include "scene.h"
#include <vector>
#include <utility>
#include <time.h>
#include "bmpgenerator.h"
using namespace std;

Scene::Scene()
{
	py_ = -1;
	px_ = -1;
	run_ = false;
}


Scene::~Scene()
{
}

int Scene::setPixels(int x, int y)
{
	if (x <= 0 || y <= 0) return -1;
	px_ = x;
	py_ = y;
	colorArray_ = std::vector<std::vector<Color>>(px_, std::vector<Color>(py_));
}

int Scene::getPixels(int* x, int* y)
{
	if (px_ == -1 || py_ == -1) return -1;
	*x = px_;
	*y = py_;
	return 0;
}

void Scene::setRootNode(std::shared_ptr<Node> node)
{
	node_ = node;
}

void Scene::setCamera(std::shared_ptr<Camera> camera)
{
	camera_ = camera;
}

void Scene::setClearColor(Color color)
{
	backGround_ = color;
}

void Scene::addLight(std::shared_ptr<Light> light)
{
	lights.push_back(light);
}

void Scene::run()
{
	srand(time(0));
	for (int x = 0; x < px_; x++) {
		if(x%10==0)
			cout << x << endl;
		for (int y = 0; y < py_; y++) {
			int n = sampleNum_;
			//jitter sample
			std::vector<std::pair<double, double>> ps;
			for (int u = 0; u < n; u++) {
				for (int v = 0; v < n; v++) {
					double ru = rand() * 1.0 / RAND_MAX + u; //rand from u to u+1
					double rv = rand() * 1.0 / RAND_MAX + v; //rand from v to v+1 
					ps.push_back(make_pair(ru / n, rv / n));
				}
			}
			//jitter light sample
			std::vector<std::pair<double, double>> ls;
			for (int u = 0; u < n; u++) {
				for (int v = 0; v < n; v++) {
					double ru = rand() * 1.0 / RAND_MAX + u; //rand from u to u+1
					double rv = rand() * 1.0 / RAND_MAX + v; //rand from v to v+1 
					ls.push_back(make_pair(ru / n, rv / n));
				}
			}
			//shuffle light sample
			for (int i = n * n - 1; i > 0; i--) {
				int rd = rand() % (i + 1);
				std::swap(ls[i],ls[rd]);
			}

			Color color;
			for (int i = 0; i < n * n; i++) {
				Ray ray = camera_->getRay((ps[i].first + x * 1.0) / px_, (ps[i].second + y * 1.0) / py_);
				for (auto& light : lights) {
					light->setSamplePosition(ls[i].first, ls[i].second);
				}
				color = color+rayColor(ray, 0, INFINITE);
			}
			colorArray_[x][y] = color * (1.0/(n * n));
			colorArray_[x][y].regularize();
		}
	}
	run_ = true;
}

int Scene::queryColor(unsigned int x, unsigned int y, Color& color)
{
	if (run_) {
		if (x >= px_ || y >= py_) return -1;
		color = colorArray_[x][y];
		return 0;
	}
	else return -1;
}

int Scene::saveToBpm(const char* filaname)
{
	if (run_) {
		BYTE* pdata = new BYTE[px_ * py_ * 3];
		for (int y = 0; y < py_; y++) {
			for (int x = 0; x < px_; x++) {
				Color* color = new Color;
				queryColor(x, y, *color);
				//cout << color->x_ << endl;
				pdata[px_ * y * 3 + x * 3 + 0] = int(color->x_ * 255);
				pdata[px_ * y * 3 + x * 3 + 1] = int(color->y_ * 255);
				pdata[px_ * y * 3 + x * 3 + 2] = int(color->z_ * 255);
				for (int i = 0; i < 3; i++) {
					//std::cout << pdata[py * x * 3 + y * 3 + i] << ' ';
				}
				//std::cout << endl;
			}
		}
		Snapshot(pdata, px_, py_, filaname);
		return 0;
	}
	return -1;
}

Color Scene::rayColor(Ray ray, double t0, double t1, int jumpTime)
{
	if (jumpTime > maxJump_) {
		return backGround_;
	}
	HitRecord rec, srec;
	if (node_->hit(ray, t0, t1, &rec)) {
		Vec3 p = ray.e + ray.d * rec.t;
		Color color;
		for (const auto& light : lights) {
			color = color + (light->getAmbient()) & (rec.mat->ambient);
			Ray newray;
			newray.e = p;
			newray.d = (light->getPosition() - p).normalize();
			//设定一个极小值为下界，防止光线与光线出发点相交，最后一个参数设为null，不关心hit信息
			if (!node_->hit(newray, ZERO, (light->getPosition() - p).length(), nullptr)) {
				color += rec.mat->diffuse & light->getDiffuse() * max(0.0, rec.normal * newray.d);
				color += rec.mat->specular & light->getSpecular() * pow((newray.d - ray.d).normalize() * rec.normal, rec.mat->shine);
			}
		}
		if (rec.mat->type == Material::SPECULAR) {
			Ray mirray;
			mirray.e = p;
			mirray.d = ray.d - rec.normal * (rec.normal * ray.d) * 2;
			//cout << rec.mat->mirror << endl;
			color += rec.mat->mirror & rayColor(mirray, ZERO, INFINITE, jumpTime + 1);
			//cout << color << endl;
		}
		return color;
	}
	else {
		return backGround_;
	}
}
