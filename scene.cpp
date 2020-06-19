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
	group_.reset(new Group);
	addChild(group_);
}


Scene::~Scene()
{
}

Scene::Scene(const Scene& scene, rt::CopyOp copyop)
	:RTObject(scene,copyop)
{
	run_ = false;
	srand(1243);
	group_.reset(new Group);
	addChild(group_);
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
	group_->addChild(node);
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
	if (auto geo = light->getGeometry()) {
		group_->addChild(geo);
	}
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

Color Scene::rayColor(const Ray& ray, double t0, double t1, int jumpTime, bool E)
{
	if (++jumpTime > maxJump_) {
		return backGround_;
	}
	HitRecord rec, srec;
	rec.ray = ray;
	//TODO check all & symbol
	if (group_->calHit(ray, t0, t1, &rec)) {
		Vec3 p = ray.e + ray.d * rec.t;
		Color color;
		if (rec.mat->getType() & Material::MONTECARLO) {
			Color c = rec.mat->getColor();
			//俄罗斯轮盘赌
			if (jumpTime > rusJump_) {
				double prosibility = c.x_ > c.y_ && c.x_ > c.z_ ? c.x_ : c.y_ > c.z_ ? c.y_ : c.z_;
				if (algorithm::rand01() < prosibility) c = c * (1 / prosibility);
				else return rec.mat->getEmission()*E; //在后续jump中消除直接光照
			}
			if (rec.mat->getType() & Material::NORMAL) {
				//计算随机间接反射方向
				//TODO 下面的这个rand01需要放在材质里，用jullit伪随机
				double r1 = PI * 2 * algorithm::rand01();
				double r2 = algorithm::rand01(), r2s = sqrt(r2);
				Vec3 w = rec.normal;
				Vec3 u = ((fabs(w.x_) > 0.1 ? Vec3(0, 1, 0) : Vec3(1, 0, 0)) ^ w).normalize();
				Vec3 v = w ^ u;
				Vec3 d = (u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2)).normalize();
				//直接光照部分
				for (const auto& light : lights) {
					double prosibility;
					Vec3 normal;
					Vec3 lightpos = light->getPosition(&rec,&normal,&prosibility);
					Ray newray;
					double dist = (lightpos - p).length();
					newray.e = p;
					newray.d = (lightpos - p).normalize();
					double dot = normal*-newray.d;
					double dot2 = rec.normal.normalize() * newray.d;
					if (dot < 0 || dot2 < 0) continue;
					//WARNING 这里可能会和光源模型本身相交，因此减去一个ZERO，但可能不够大
					if (!group_->calHit(newray, ZERO, dist-ZERO, nullptr)) {
						//TODO 此处后面一项是双边辐射函数，后续要作为material的虚函数返回
						color = color + (c & light->getDiffuse() *(dot)*(dot2)*(1/prosibility/dist/dist));
					}
				}
				//std::cout << color;
				color = rec.mat->getEmission()*E + color + (rec.mat->getColor() & rayColor(Ray{ p, d }, ZERO, INFINITE, jumpTime + 1, 0));
				//std::cout << color;
			}
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
				//TODO 如果
				color += rec.mat->getEmission()*E +( rec.mat->getColor() & rayColor(mirray, ZERO, INFINITE, jumpTime));
			}
			if (rec.mat->getType() & Material::TRANSPARENT) {
				Ray refRay;
				refRay.e = p;
				Color k(1, 1, 1, 1); //光强损失ko
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
						color += rec.mat->getEmission()*E + (k & rayColor(Ray{ p,r }, ZERO, INFINITE, jumpTime));
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

					//TODO加上俄罗斯轮盘赌， 全反射和折射要乘上mat->Color()
					color += (k & (rayColor(Ray{ p,r }, ZERO, INFINITE, jumpTime)*R) + rayColor(refRay, ZERO, INFINITE, jumpTime)*(1 - R));
				}
			}
		}
		else {
			//漫反射材质
			if (rec.mat->getType() & Material::NORMAL) {
				for (const auto& light : lights) {
					color = color + ((light->getAmbient()) & (rec.mat->getAmbient(&rec)));
					Ray newray;
					newray.e = p;
					Vec3 lightpos = light->getPosition();
					newray.d = (lightpos - p).normalize();
					//设定一个极小值为下界，防止光线与光线出发点相交，最后一个参数设为null，不关心hit信息
					if (!group_->calHit(newray, ZERO, (lightpos - p).length() - ZERO, nullptr)) {
						color += rec.mat->getDiffuse(&rec) & light->getDiffuse() * max(0.0, rec.normal * newray.d);
						color += rec.mat->getSpecular(&rec) & light->getSpecular() * pow((newray.d - ray.d).normalize() * rec.normal, rec.mat->getShine());
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
				color += rec.mat->getMirror() & rayColor(mirray, ZERO, INFINITE, jumpTime);
			}
			//折射材质
			if (rec.mat->getType() & Material::TRANSPARENT) {
				Ray refRay;
				refRay.e = p;
				Color k(1, 1, 1, 1); //光强损失ko
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
						color += k & rayColor(Ray{ p,r }, ZERO, INFINITE, jumpTime);
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
					color += k & (rayColor(Ray{ p,r }, ZERO, INFINITE, jumpTime)*R + rayColor(refRay, ZERO, INFINITE, jumpTime)*(1 - R));
				}
			}
		}
		//assert(color.x_ >= 0 && color.y_ >= 0 && color.z_ >= 0);
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