#include <direct.h>
#include <io.h>
#include <string>
#include <sstream>
#include "scene.h"
#include "AviSaver.h"
#include "bmpgenerator.h"
#include "types.h"

//#define ANIMATION
//#define HIGHQUALITY

#ifdef HIGHQUALITY
const int px = 1920;
const int py = 1080;
//const int px = 1280;
//const int py = 720;
#else
const int px = 1020;
const int py = 630;
//const int px = 640;
//const int py = 360;
#endif // HIGHQUALITY

using namespace std;

//TODO 630X1020 画面中间有缝

int main() {
	double R = 300;
	double T = 30 * PI / 180.;
	double D = R / cos(T);
	double Z = 62;
	Color r = Color(1.2, 1, 1, 1);
	Color g = Color(1, 1.1, 1, 1);
	Color b = Color(1, 1, 1, 1);

	std::shared_ptr<Group> group(new Group);
	std::shared_ptr<Sphere> back(new Sphere(Vec3(50, 28, Z) + Vec3(0, 0, -1)*R * 2 * sqrt(2. / 3.), R));
	std::shared_ptr<Sphere> front(new Sphere(Vec3(50, 28, Z) + Vec3(0, 0, -R * 2 * sqrt(2. / 3.) / 3.), 2 * 2 * R * 2 * sqrt(2. / 3.) - R * 2 * sqrt(2. / 3.) / 3.));
	group->addChild(back);
	group->addChild(front);
	//设置材质
	Color C = Color(0.3, 0.7, 0.8,1);
	double emission = 0.8e-1;
	std::shared_ptr<Material> mr(new Material(C*emission&r, Color(1.15, .996, .996, 0), Material::SPECULAR));
	std::shared_ptr<Material> mg(new Material(C*emission&g, Color(.99, 1.03, .99, 0), Material::SPECULAR));
	std::shared_ptr<Material> mb(new Material(C*emission&b, Color(.996, .996, .996, 0), Material::SPECULAR));
	std::shared_ptr<Material> mback(new Material(C*0e-2, Color(.95, .95, .95, 0), Material::SPECULAR));
	std::shared_ptr<Material> mfront(new Material(C*0e-2, Color(.93, .93, .93, 0), Material::SPECULAR));
	
	//设置场景
	std::shared_ptr<Scene> scene(new Scene());
	scene->setPixels(px, py);
	scene->setClearColor(Vec4(0.0, 0.0, 0.0, 1.0));
	std::shared_ptr<Camera> camera(new Camera(Vec3(50, 40, -120), Vec3(50, 40, Z), Vec3(0, 1, 0), 95, px * 1.0 / py));
	std::shared_ptr<SphereLight> lr(new SphereLight(Vec3(50, 28, Z) + Vec3(cos(T), sin(T), 0)*D,R, C*emission&r));
	std::shared_ptr<SphereLight> lg(new SphereLight(Vec3(50, 28, Z) + Vec3(-cos(T), sin(T), 0)*D, R, C*emission&g));
	std::shared_ptr<SphereLight> lb(new SphereLight(Vec3(50, 28, Z) + Vec3(0,-1, 0)*D, R, C*emission&b));
	
	//绑定材质
	back->setMaterial(mback);
	front->setMaterial(mfront);
	lr->getGeometry()->setMaterial(mr);
	lg->getGeometry()->setMaterial(mg);
	lb->getGeometry()->setMaterial(mb);

	scene->setRootNode(group);
	scene->setCamera(camera);
	scene->addLight(lr);
	scene->addLight(lg);
	scene->addLight(lb);

#ifdef  ANIMATION
	double time = 0;
	std::shared_ptr<AviSaver> avi;
	if (0 != _access("./test", 0)) {
		_mkdir("./test");
	}
	avi.reset(new AviSaver());
	avi->Open("./test/test.avi", px, py, 15, 1);

	for (int count = 0; time <= 2; time += 0.015, count++) {
		std::cout << "time " << time << std::endl;
		scene->setTime(time);
		scene->runParallelly(11);
		BYTE* allData = new BYTE[px*py * 3];;

		for (int i = 0; i < py; i++) {
			for (int j = 0; j < px; j++) {
				Color color;
				scene->queryColor(j, i, color);
				allData[i*px * 3 + j * 3 + 2] = int(color.x_ * 255);
				allData[i*px * 3 + j * 3 + 1] = int(color.y_ * 255);
				allData[i*px * 3 + j * 3 + 0] = int(color.z_ * 255);
			}
		}
		avi->AddImg(allData);
	}
	avi->Save();
#else
	scene->runParallelly(8);

	if (0 != _access("./test", 0)) {
		_mkdir("./test");
	}
	scene->saveToBpm(std::string("./test/test.bmp").c_str());
#endif
}