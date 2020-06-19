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
	//设置模型
	std::shared_ptr<Group> group(new Group);
	//std::shared_ptr<HeartShape> sphere(new HeartShape(Vec3(1, 4.0, 1), 1.8));
	//std::shared_ptr<SurfaceOfGenus2> genus2(new SurfaceOfGenus2(Vec3(1, 1.3, 1), 1.3));
	std::shared_ptr<Sphere> mirrorsphere1(new Sphere(Vec3(3, 1.5, -2), 1.5)); //mirror sphere
	std::shared_ptr<Sphere> mirrorsphere2(new Sphere(Vec3(-4, 1.5, 2), 1.5)); //mirror sphere
	//std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.2, 5), 1.2));
	std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.4, 5), 1.4));
	std::shared_ptr<Ground> ground(new Ground(0));
	std::shared_ptr<Wall_z> fwall(new Wall_z(-10));
	std::shared_ptr<Wall_x> lwall(new Wall_x(-15));
	//设置材质
	//std::shared_ptr<Material> nsmat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));  //普通材质
	//std::shared_ptr<Material> nsmat(new PerlinNoiseNormalMaterial_Blood(Color(1.2, 1.0, 1.0, 1.0), Color(1.0, .9, .9, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f, 0.3));  //猩红材质
	std::shared_ptr<Material> nsmat(new Material(Color(1.0, 0, 0, 1.0), Color(1.0, 0, 0, 1.0), Color(0.9, 0.9, 0.9, 1.0), 32.0f));  //火红材质
	std::shared_ptr<Material> msmat(new Material(Color(),Color(.99,.99,.99,0),Material::SPECULAR)); //镜面材质
	std::shared_ptr<Material> msmat_blur(new Material(Color(0.4, 0.45, 0.6, 1.0), Color(0.4, 0.45, 0.6, 1.0), Color(1.0, 1.0, 1.0, 1.0), Color(0.4, 0.4, 0.4, 1.0), 512.0f));
	std::shared_ptr<Material> genus2mat(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.8, 0.95, 1.0), Color(0.8, 0.8, 0.95, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.5));
	msmat_blur->setMirrorBlur(0.05);
	std::shared_ptr<Material> transMat(new Material(Color(0.07, 0.05, 0.1, 1), 1.4));
	transMat->setMonteCarlo(Color(.99, .99, .99, 0));
	std::shared_ptr<Material> gmat(new Material(Color(),Color(0.75, 0.75 ,0.75,1.0),Material::NORMAL));
	std::shared_ptr<Material> wmat1(new Material(Color(), Color(0.25, 0.25, 0.75, 1.0), Material::NORMAL));
	std::shared_ptr<Material> wmat2(new Material(Color(), Color(0.75, 0.25, 0.25, 1.0), Material::NORMAL));
	/*std::shared_ptr<Material> gmat(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	std::shared_ptr<Material> wmat1(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	std::shared_ptr<Material> wmat2(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));*/
	//绑定材质
	//sphere->setMaterial(nsmat);
	mirrorsphere1->setMaterial(msmat);
	mirrorsphere2->setMaterial(msmat);
	//genus2->setMaterial(msmat);
	transsphere->setMaterial(transMat);
	ground->setMaterial(gmat);
	fwall->setMaterial(wmat1);
	lwall->setMaterial(wmat2);
	//绑定模型
	//group->addChild(sphere);
	//group->addChild(genus2);
	group->addChild(mirrorsphere1);
	group->addChild(mirrorsphere2);
	group->addChild(transsphere);
	group->addChild(ground);
	group->addChild(fwall);
	group->addChild(lwall);
	//设置场景
	std::shared_ptr<Scene> scene(new Scene());
	scene->setPixels(px, py);
	scene->setClearColor(Vec4(0.0, 0.0, 0.0, 1.0));
	std::shared_ptr<Camera> camera(new Camera(Vec3(5, 7, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 75, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new Camera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 17.0, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new DepthCamera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 45.0, px * 1.0 / py, 0.5f));
#ifdef ANIMATION
	std::shared_ptr<MovableFaceLight> facelight1(new MovableFaceLight(Vec3(5, 5, 3), Vec3(-5, -5, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.5, 0.5, 0.5, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#else
	//std::shared_ptr<FaceLight> facelight1(new FaceLight(Vec3(5, 20, 2), Vec3(-5, -5, -5).normalize(), Vec3(0, 1, 0), 15.0, 15.0,
	//	Color(0.2, 0.2, 0.2, 1.0), Color(0.7, 0.7, 0.7, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#endif
	//std::shared_ptr<FaceLight> facelight2(new FaceLight(Vec3(5, 20, -2), Vec3(-10, -10, 10).normalize(), Vec3(0, 1, 0), 15.0, 15.0,
	//	Color(0.2, 0.2, 0.2, 1.0), Color(1, 1, 1, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
	std::shared_ptr<SphereLight> light1(new SphereLight(Vec3(5, 8, -4), 0.6, Color(60, 60, 60, 1)));
	std::shared_ptr<SphereLight> light2(new SphereLight(Vec3(-3,8,4),0.6,Color(80,80,80,1)));
	scene->setCamera(camera);
	scene->setRootNode(group);
	scene->addLight(light1);
	scene->addLight(light2);
	double time = 0;

#ifdef  ANIMATION
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
	scene->runParallelly(11);

	if (0 != _access("./test", 0)) {
		_mkdir("./test");
	}
	scene->saveToBpm(std::string("./test/test.bmp").c_str());
#endif
}