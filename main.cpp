#include "scene.h"
using namespace std;
//const int px = 1280;
//const int py = 720;
//const int px = 720;
//const int py = 480;
const int px = 1920;
const int py = 1080;
//const int px = 5120;
//const int py = 2880;
int main() {
	std::shared_ptr<Group> group(new Group);
	std::shared_ptr<Sphere> sphere(new Sphere(Vec3(1,1.5,1.5),1.5)); //normal sphere
	std::shared_ptr<Sphere> mirrorsphere1(new Sphere(Vec3(3, 1.5, -2), 1.5)); //mirror sphere
	std::shared_ptr<Sphere> mirrorsphere2(new Sphere(Vec3(-3, 1.5, 2), 1.5)); //mirror sphere
	std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.2, 5), 1.2)); //mirror sphere
	std::shared_ptr<Ground> ground(new Ground(0));
	std::shared_ptr<Wall_z> fwall(new Wall_z(-10));
	std::shared_ptr<Wall_x> lwall(new Wall_x(-10));

	std::shared_ptr<Material> nsmat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));
	std::shared_ptr<Material> msmat(new Material(Color(0.0, 0.0, 0.0, 1.0), Color(0.0, 0.0, 0.0, 1.0), Color(1.2, 1.2 ,1.2, 1.0), Color(1, 1, 1, 1.0), 512.0f));
	std::shared_ptr<Material> msmat_blur(new Material(Color(0.6, 0.45, 0.4, 1.0), Color(0.6, 0.45, 0.4, 1.0), Color(1.0, 1.0, 1.0, 1.0), Color(0.4, 0.4, 0.4, 1.0), 512.0f));
	msmat_blur->setMirrorBlur(0.1);
	std::shared_ptr<Material> transMat(new Material(Color(0.1, 0.05, 0.1, 1), 1.5));
	std::shared_ptr<Material> gmat(new Material(Color(0.9,1,0.9,1.0),Color(0.9,1,0.9,1.0),Color(0.2,0.2,0.2,1.0),15.0f));
	std::shared_ptr<Material> wmat(new Material(Color(0.6,0.6,0.6,1.0),Color(0.6,0.6,0.6,1.0),Color(0.1,0.1,0.1,1.0),10.0f));

	sphere->setMaterial(nsmat);
	mirrorsphere1->setMaterial(msmat);
	mirrorsphere2->setMaterial(msmat_blur);
	transsphere->setMaterial(transMat);
	ground->setMaterial(gmat);
	fwall->setMaterial(wmat);
	lwall->setMaterial(wmat);

	group->addChild(sphere);
	group->addChild(mirrorsphere1);
	group->addChild(mirrorsphere2);
	group->addChild(transsphere);
	group->addChild(ground);
	group->addChild(fwall);
	group->addChild(lwall);

	Scene scene;
	scene.setPixels(px,py);
	scene.setClearColor(Vec4(0.9, 0.9, 0.9, 1.0));
	std::shared_ptr<Camera> camera(new Camera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 45.0, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new DepthCamera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 45.0, px * 1.0 / py, 0.5f));
	std::shared_ptr<FaceLight> facelight1(new FaceLight(Vec3(10, 10, 3), Vec3(-10, -10, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.1, 0.1, 0.1, 1.0), Color(0.5, 0.5, 0.5, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
	std::shared_ptr<FaceLight> facelight2(new FaceLight(Vec3(5, 5, -5), Vec3(-10, -10, 10).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.1, 0.1, 0.1, 1.0), Color(0.4, 0.4, 0.4, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
	scene.setCamera(camera);
	scene.setRootNode(group);
	scene.addLight(facelight1);
	scene.addLight(facelight2);

	std::cout << Vec3(1.0, 1.0, 1.0).reflact(Vec3(0, 0, 1)) << std::endl;

	scene.run();

	scene.saveToBpm("./test.bmp");
}
