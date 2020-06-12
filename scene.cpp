#include "scene.h"
#include <vector>
#include <utility>
#include <time.h>
#include <omp.h>
#include "bmpgenerator.h"
#include "algorithms.h"
#include "cudaRayTracer.h"
using namespace std;
using namespace rt;

int refract(Vec3 d, Vec3 normal, double n, Vec3* t);

Scene::Scene()
{
	py_ = -1;
	px_ = -1;
	run_ = false;
	setRandomEnabled(true);
	setRandomSize(sampleNum_, sampleNum_);
	srand(1243);
}


Scene::~Scene()
{
}

Scene::Scene(const Scene& scene, rt::CopyOp copyop)
	:RTObject(scene,copyop)
{
	run_ = false;
	srand(1243);
	switch (copyop)
	{
	case RECUR_SHALLOW_COPY:
		setPixels(scene.px_, scene.py_);
		setRootNode(scene.node_->cloneToSharedPtr(RECUR_SHALLOW_COPY));
		setCamera(scene.camera_->cloneToSharedPtr(RECUR_SHALLOW_COPY));
		setClearColor(scene.backGround_);
		for (const auto& light : scene.lights) {
			addLight(light->cloneToSharedPtr(RECUR_SHALLOW_COPY));
		}
	}
}

int Scene::setPixels(int x, int y)
{
	if (x <= 0 || y <= 0) return -1;
	px_ = x;
	py_ = y;
	colorArray_ = std::vector<std::vector<Color>>(px_, std::vector<Color>(py_,Color(-1,-1,-1,-1)));
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
	addChild(node);
}

void Scene::setCamera(std::shared_ptr<Camera> camera)
{
	camera_ = camera;
	addChild(camera);
}

void Scene::setClearColor(Color color)
{
	backGround_ = color;
}

void Scene::addLight(std::shared_ptr<Light> light)
{
	lights.push_back(light);
	addChild(light);
}

void Scene::run(int x, int y)
{
	if (x >= 0 && y >= 0)
	{
		calRandom();
		int n = sampleNum_;
		Ray ray;
		Color color;
		for (int i = 0; i < n * n; i++) {
			setIteratorIndex(i);
			std::pair<double, double> pair;
			getRandomPair(&pair);
			ray = camera_->getRay((pair.first + x * 1.0) / px_, (pair.second + y * 1.0) / py_);
			color = color + rayColor(ray, 0, INFINITE);
		}
		colorArray_[x][y] = color * (1.0 / (n * n));
		colorArray_[x][y].regularize();
	}
	else {
		for (int x = 0; x < px_; x++) {
			if (x % 10 == 0)
				cout << x << endl;
			for (int y = 0; y < py_; y++) {
				//recaculate the random array before every iterator
				calRandom();
				int n = sampleNum_;
				Ray ray;
				Color color;
				for (int i = 0; i < n * n; i++) {
					setIteratorIndex(i);
					std::pair<double, double> pair;
					getRandomPair(&pair);
					ray = camera_->getRay((pair.first + x * 1.0) / px_, (pair.second + y * 1.0) / py_);
					color = color + rayColor(ray, 0, INFINITE);
				}

				colorArray_[x][y] = color * (1.0 / (n * n));
				colorArray_[x][y].regularize();
			}
		}
	}
	run_ = true;
}

void Scene::runParallelly(int threadNum)
{
	int nthreads, tid, count = 0;
	omp_set_num_threads(threadNum);
#pragma omp parallel private(nthreads,tid) num_threads(threadNum)
	{
		std::shared_ptr<Scene> scene;
		tid = omp_get_thread_num();
		nthreads = omp_get_num_threads();
		if(tid) scene = this->cloneToSharedPtr(RECUR_SHALLOW_COPY);
		else scene = std::dynamic_pointer_cast<Scene>(this->shared_from_this());
		setRandomEnabled(true);
		int tot = px_ * py_;
		while (count != tot) {
			int thiscount;
			#pragma omp critical
			{
				thiscount = count;
				count++;
			}
			int y = thiscount / px_;
			int x = thiscount - y * px_;
			scene->run(x, y);
			if (thiscount % (10*px_) == 0) cout << (thiscount*100.0/tot) <<"%" << endl;
			Color color;
			scene->queryColor(x,y,color);
			#pragma omp critical
			this->setPixelColor(x, y, color);
		}
	}
	run_ = true;
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
				pdata[px_ * y * 3 + x * 3 + 2] = int(color->x_ * 255);
				pdata[px_ * y * 3 + x * 3 + 1] = int(color->y_ * 255);
				pdata[px_ * y * 3 + x * 3 + 0] = int(color->z_ * 255);
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
	if (node_->calHit(ray, t0, t1, &rec)) {
		Vec3 p = ray.e + ray.d * rec.t;
		Color color;
		//漫反射材质
		if (rec.mat->getType() & Material::NORMAL) {
			for (const auto& light : lights) {
				color = color + ((light->getAmbient()) & (rec.mat->getAmbient()));
				Ray newray;
				newray.e = p;
				newray.d = (light->getPosition() - p).normalize();
				//设定一个极小值为下界，防止光线与光线出发点相交，最后一个参数设为null，不关心hit信息
				if (!node_->calHit(newray, ZERO, (light->getPosition() - p).length(), nullptr)) {
					color += rec.mat->getDiffuse() & light->getDiffuse() * max(0.0, rec.normal * newray.d);
					color += rec.mat->getSpecular() & light->getSpecular() * pow((newray.d - ray.d).normalize() * rec.normal, rec.mat->getShine());
				}
			}
		}
		//镜面材质
		if (rec.mat->getType() & Material::SPECULAR) {
			Ray mirray;
			mirray.e = p;
			mirray.d = ray.d - rec.normal * (rec.normal * ray.d) * 2;
			if (rec.mat->getMirrorBlur() >= ZERO) {
				Vec3 vec(0, 0, 1);
				if ((vec ^ mirray.d).isZero()) vec = Vec3(0, 1, 0);
				Vec3 u = (vec ^ mirray.d).normalize();
				Vec3 v = (mirray.d ^ u).normalize();
				double x, y;
				algorithm::randFromDisk(&x, &y);
				x *= rec.mat->getMirrorBlur();
				y *= rec.mat->getMirrorBlur();
				mirray.d = mirray.d + u * x + v * y;
			}
			color += rec.mat->getMirror() & rayColor(mirray, ZERO, INFINITE, jumpTime + 1);
			//cout << color << endl;
		}
		//折射材质
		if (rec.mat->getType() & Material::TRANSPARENT) {
			Ray refRay;
			refRay.e = p;
			Color k(1,1,1,1); //光强损失
			double c; // 空气侧的cos角
			Vec3 r = ray.d.reflact(rec.normal); //反射方向
			//std::cout << ray.d << std::endl << rec.normal << std::endl << r << std::endl;
			bool flag = false; // 是否全反射
			 /*介质侧射出*/
			if (rec.normal * ray.d > 0) { 

				k.x_ = exp(-rec.mat->getAttenuation().x_*rec.t);
				k.y_ = exp(-rec.mat->getAttenuation().y_*rec.t);
				k.z_ = exp(-rec.mat->getAttenuation().z_*rec.t);
				if (refract(ray.d, -rec.normal, 1 / rec.mat->getRefraCoef(), &refRay.d)) { //全反射
					flag = true;
					color += k & rayColor(Ray{p,r}, ZERO, INFINITE, jumpTime + 1);
				}
				else {
					c = refRay.d * rec.normal;
				}
			}
			else { //空气侧射入
				c = -ray.d * rec.normal;
				k.x_ = k.y_ = k.z_ = 1.0; // 没有光强损失
				refract(ray.d, rec.normal, rec.mat->getRefraCoef(), &refRay.d);
			}
			if (!flag) {
				double n = rec.mat->getRefraCoef();
				double R0 = (n - 1)*(n - 1) / (n + 1) / (n + 1); //TODO 优化该计算过程到material中
				double R = R0 + (1 - R0)*pow(1 - c, 5);
				//std::cout <<  R << std::endl;
				color += k & (rayColor(Ray{ p,r }, ZERO, INFINITE, jumpTime + 1)*R + rayColor(refRay, ZERO, INFINITE, jumpTime + 1)*(1-R));
			}
		}
		return color;
	}
	else {
		return backGround_;
	}
}

//返回：0 折射； -1 全反射
int refract(Vec3 d, Vec3 normal, double n, Vec3* t) {
	double inSqrt = 1 - (1 - (d*normal)*(d*normal)) / n / n;
	if (inSqrt < 0) {
		*t = d.reflact(normal);
		return -1; //全反射
	}
	else {
		*t = (d - normal * (d*normal)) * (1/n) - normal * sqrt(inSqrt);
		*t = t->normalize();
		return 0;
	}
}