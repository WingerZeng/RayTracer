#include <direct.h>
#include <io.h>
#include <string>
#include <sstream>
#include "scene.h"
#include "AviSaver.h"
#include "mpi.h"
#include "bmpgenerator.h"
#include "types.h"

//#define ANIMATION
#define HIGHQUALITY

#ifdef HIGHQUALITY
const int px = 1280;
const int py = 720;
#else
//const int px = 1020;
//const int py = 630;
const int px = 640;
const int py = 360;
#endif // HIGHQUALITY

using namespace std;

int main() {
	//设置模型
	std::shared_ptr<Group> group(new Group);
	std::shared_ptr<HeartShape> sphere(new HeartShape(Vec3(1, 1.5, 1.8), 1.8)); //normal sphere
	std::shared_ptr<Sphere> mirrorsphere1(new Sphere(Vec3(3, 1.5, -2), 1.5)); //mirror sphere
	std::shared_ptr<Sphere> mirrorsphere2(new Sphere(Vec3(-3, 1.5, 2), 1.5)); //mirror sphere
	//std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.2, 5), 1.2));
	std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.4, 5), 1.4));
	std::shared_ptr<Ground> ground(new Ground(0));
	std::shared_ptr<Wall_z> fwall(new Wall_z(-10));
	std::shared_ptr<Wall_x> lwall(new Wall_x(-10));
	//设置材质
	//std::shared_ptr<Material> nsmat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));
	std::shared_ptr<Material> nsmat(new PerlinNoiseNormalMaterial_Blood(Color(1.2, 1, 1, 1.0), Color(1.0, 0.9, 0.9, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f, 0.3));
	std::shared_ptr<Material> msmat(new Material(Color(0.0, 0.0, 0.0, 1.0), Color(0.0, 0.0, 0.0, 1.0), Color(1.2, 1.2, 1.2, 1.0), Color(1, 1, 1, 1.0), 512.0f));
	std::shared_ptr<Material> msmat_blur(new Material(Color(0.4, 0.45, 0.6, 1.0), Color(0.4, 0.45, 0.6, 1.0), Color(1.0, 1.0, 1.0, 1.0), Color(0.4, 0.4, 0.4, 1.0), 512.0f));
	msmat_blur->setMirrorBlur(0.05);
	std::shared_ptr<Material> transMat(new Material(Color(0.07, 0.05, 0.1, 1), 1.4));
	//std::shared_ptr<Material> transMat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));
	std::shared_ptr<Material> gmat(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	//std::shared_ptr<Material> gmat(new Material(Color(0.9, 1, 0.9, 1.0), Color(0.9, 1, 0.9, 1.0), Color(0.2, 0.2, 0.2, 1.0), 15.0f));
	std::shared_ptr<Material> wmat1(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	std::shared_ptr<Material> wmat2(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	//std::shared_ptr<Material> wmat(new Material(Color(0.6, 0.6, 0.6, 1.0), Color(0.6, 0.6, 0.6, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f));
	//绑定材质
	sphere->setMaterial(nsmat);
	mirrorsphere1->setMaterial(msmat);
	mirrorsphere2->setMaterial(msmat_blur);
	transsphere->setMaterial(transMat);
	ground->setMaterial(gmat);
	fwall->setMaterial(wmat1);
	lwall->setMaterial(wmat2);
	//绑定模型
	group->addChild(sphere);
	group->addChild(mirrorsphere1);
	group->addChild(mirrorsphere2);
	group->addChild(transsphere);
	group->addChild(ground);
	group->addChild(fwall);
	group->addChild(lwall);
	//设置场景
	std::shared_ptr<Scene> scene(new Scene());
	scene->setPixels(px, py);
	scene->setClearColor(Vec4(0.9, 0.9, 0.9, 1.0));
	std::shared_ptr<Camera> camera(new Camera(Vec3(5, 7, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 70, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new Camera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 17.0, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new DepthCamera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 45.0, px * 1.0 / py, 0.5f));
#ifdef ANIMATION
	std::shared_ptr<MovableFaceLight> facelight1(new MovableFaceLight(Vec3(5, 5, 3), Vec3(-5, -5, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.5, 0.5, 0.5, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#else
	std::shared_ptr<FaceLight> facelight1(new FaceLight(Vec3(5, 5, 4), Vec3(-5, -5, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.10, 0.10, 0.10, 1.0), Color(0.50, 0.50, 0.55, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#endif
	std::shared_ptr<FaceLight> facelight2(new FaceLight(Vec3(5, 5, -3), Vec3(-10, -10, 10).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.45, 0.45, 0.45, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
	scene->setCamera(camera);
	scene->setRootNode(group);
	scene->addLight(facelight1);
	scene->addLight(facelight2);
	double time = 0;

	scene->runParallelly(11);
	if (0 != _access("./test", 0)) {
		_mkdir("./test");
	}
	scene->saveToBpm(std::string("./test/test.bmp").c_str());
}

//MPI 并行方式,已废弃
#if 0
int main(int argc,char*argv[]) {
	//初始化MPI
	MPI_Init(&argc, &argv);
	int id, p;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	//设置模型
	std::shared_ptr<Group> group(new Group);
	std::shared_ptr<HeartShape> sphere(new HeartShape(Vec3(1,1.5,1.8), 1.8)); //normal sphere
	std::shared_ptr<Sphere> mirrorsphere1(new Sphere(Vec3(3, 1.5, -2), 1.5)); //mirror sphere
	std::shared_ptr<Sphere> mirrorsphere2(new Sphere(Vec3(-3, 1.5, 2), 1.5)); //mirror sphere
	//std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.2, 5), 1.2));
	std::shared_ptr<Sphere> transsphere(new Sphere(Vec3(0, 1.4, 5),1.4));
	std::shared_ptr<Ground> ground(new Ground(0));
	std::shared_ptr<Wall_z> fwall(new Wall_z(-10));
	std::shared_ptr<Wall_x> lwall(new Wall_x(-10));
	//设置材质
	//std::shared_ptr<Material> nsmat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));
	std::shared_ptr<Material> nsmat(new PerlinNoiseNormalMaterial_Blood(Color(1.2, 1, 1, 1.0), Color(1.0, 0.9, 0.9, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f,0.3));
	std::shared_ptr<Material> msmat(new Material(Color(0.0, 0.0, 0.0, 1.0), Color(0.0, 0.0, 0.0, 1.0), Color(1.2, 1.2 ,1.2, 1.0), Color(1, 1, 1, 1.0), 512.0f));
	std::shared_ptr<Material> msmat_blur(new Material(Color(0.4, 0.45, 0.6, 1.0), Color(0.4, 0.45, 0.6, 1.0), Color(1.0, 1.0, 1.0, 1.0), Color(0.4, 0.4, 0.4, 1.0), 512.0f));
	msmat_blur->setMirrorBlur(0.05);
	std::shared_ptr<Material> transMat(new Material(Color(0.07, 0.05, 0.1, 1), 1.4));
	//std::shared_ptr<Material> transMat(new Material(Color(0.6, 0.6, 0.8, 1.0), Color(0.6, 0.6, 0.8, 1.0), Color(0.9, 0.9, 0.9, 1.0), 70.0f));
	std::shared_ptr<Material> gmat(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	//std::shared_ptr<Material> gmat(new Material(Color(0.9, 1, 0.9, 1.0), Color(0.9, 1, 0.9, 1.0), Color(0.2, 0.2, 0.2, 1.0), 15.0f));
	std::shared_ptr<Material> wmat1(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0),Color(0.8, 0.95, 0.8, 1.0),Color(0.1,0.1,0.1,1.0),10.0f,0.8));
	std::shared_ptr<Material> wmat2(new PerlinNoiseNormalMaterial_Blood(Color(0.8, 0.95, 0.8, 1.0), Color(0.8, 0.95, 0.8, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f, 0.8));
	//std::shared_ptr<Material> wmat(new Material(Color(0.6, 0.6, 0.6, 1.0), Color(0.6, 0.6, 0.6, 1.0), Color(0.1, 0.1, 0.1, 1.0), 10.0f));
	//绑定材质
	sphere->setMaterial(nsmat);
	mirrorsphere1->setMaterial(msmat);
	mirrorsphere2->setMaterial(msmat_blur);
	transsphere->setMaterial(transMat);
	ground->setMaterial(gmat);
	fwall->setMaterial(wmat1);
	lwall->setMaterial(wmat2);
	//绑定模型
	group->addChild(sphere);
	group->addChild(mirrorsphere1);
	group->addChild(mirrorsphere2);
	group->addChild(transsphere);
	group->addChild(ground);
	group->addChild(fwall);
	group->addChild(lwall);
	//设置场景
	Scene scene;
	scene.setPixels(px,py);
	scene.setClearColor(Vec4(0.9, 0.9, 0.9, 1.0));
	std::shared_ptr<Camera> camera(new Camera(Vec3(5, 7, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 75.0, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new Camera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 17.0, px * 1.0 / py));
	//std::shared_ptr<Camera> camera(new DepthCamera(Vec3(5, 5, 15), Vec3(1, 1.5, 1.5), Vec3(0, 1, 0), 45.0, px * 1.0 / py, 0.5f));
#ifdef ANIMATION
	std::shared_ptr<MovableFaceLight> facelight1(new MovableFaceLight(Vec3(5, 5, 3), Vec3(-5, -5, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.5, 0.5, 0.5, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#else
	std::shared_ptr<FaceLight> facelight1(new FaceLight(Vec3(5, 5, 5), Vec3(-5, -5, 0).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.45, 0.45, 0.55, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
#endif

	std::shared_ptr<FaceLight> facelight2(new FaceLight(Vec3(5, 5, -7.5), Vec3(-10, -10, 10).normalize(), Vec3(0, 1, 0), 5.0, 5.0,
		Color(0.2, 0.2, 0.2, 1.0), Color(0.45, 0.45, 0.45, 1.0), Color(0.8, 0.8, 0.8, 1.0)));
	scene.setCamera(camera);
	scene.setRootNode(group);
	scene.addLight(facelight1);
	scene.addLight(facelight2);
	double time = 0;

#ifdef  ANIMATION
	std::shared_ptr<AviSaver> avi;
	if (!id) {
		if (0 != _access("./test", 0)) {
			_mkdir("./test");
		}
		avi.reset(new AviSaver());
		avi->Open("./test/test.avi", px, py, 15, 1);
	}
	for (int count =0; time <= 1.5; time += 0.02,count++) {
		if (!id) std::cout << "time " << time << std::endl;
		scene.setTime(time);
		int x, y;
		scene.getPixels(&x, &y);
		int begin = y * id / p;
		int end = y * (id + 1) / p;
		int size = end - begin;
		BYTE* pdata = new BYTE[size * x * 3];
		//std::cout << begin << ' ' << end << endl;
		for (int i = begin, count = 0; i < end; i++, count++) {
			if (count % 20 == 0) {
				//std::cout << "process " << id << " finish " << count << endl;
			}
			for (int j = 0; j < x; j++) {
				scene.run(j, i);
				Color color;
				scene.queryColor(j, i, color);
				pdata[count*x * 3 + j * 3 + 2] = int(color.x_ * 255);
				pdata[count*x * 3 + j * 3 + 1] = int(color.y_ * 255);
				pdata[count*x * 3 + j * 3 + 0] = int(color.z_ * 255);
				//cout << size * x * 3 << ' ' << count * x * 3 + j * 3 + 2 << std::endl;
			}
		}
		BYTE* allData;
		int* recv_cnt;
		int* recv_disp;
		if (!id) {
			allData = new BYTE[x*y * 3];
			recv_cnt = new int[p];
			recv_disp = new int[p];
			recv_disp[0] = 0;
			bool first = true;
			for (int i = 0; i < p; i++) {
				recv_cnt[i] = (y * (i + 1) / p - y * i / p)*x * 3;
				if (first) first = false;
				else recv_disp[i] = recv_disp[i - 1] + recv_cnt[i - 1];
			}
		}
		//std::cout << "process " << id << " finish " << endl;
		MPI_Gatherv(pdata, size*x * 3, MPI_CHAR, allData, recv_cnt, recv_disp, MPI_CHAR, 0, MPI_COMM_WORLD);
		//std::cout << " finish " << endl;

		if (!id) {
			//save single pic
			//std::stringstream sio;
			//sio << count;
			//string str;
			//sio >> str;
			//Snapshot(allData, x, y, (std::string("./test/test")+ str +".bmp").c_str());

			avi->AddImg(allData);
		}
	}
	if (!id) {
		avi->Save();
	}
#else
	if (!id) {
		if (0 != _access("./test", 0)) {
			_mkdir("./test");
		}
	}
	{
		//scene.setTime(time);
		int x, y;
		scene.getPixels(&x, &y);
		int begin = y * id / p;
		int end = y * (id + 1) / p;
		int size = end - begin;
		BYTE* pdata = new BYTE[size * x * 3];
		//std::cout << begin << ' ' << end << endl;
		for (int i = begin, count = 0; i < end; i++, count++) {
			if (count % 20 == 0) {
				std::cout << "process " << id << " finish " << count << endl;
			}
			for (int j = 0; j < x; j++) {
				scene.run(j, i);
				Color color;
				scene.queryColor(j, i, color);
				pdata[count*x * 3 + j * 3 + 2] = int(color.x_ * 255);
				pdata[count*x * 3 + j * 3 + 1] = int(color.y_ * 255);
				pdata[count*x * 3 + j * 3 + 0] = int(color.z_ * 255);
				//cout << size * x * 3 << ' ' << count * x * 3 + j * 3 + 2 << std::endl;
			}
		}
		BYTE* allData;
		int* recv_cnt;
		int* recv_disp;
		if (!id) {
			allData = new BYTE[x*y * 3];
			recv_cnt = new int[p];
			recv_disp = new int[p];
			recv_disp[0] = 0;
			bool first = true;
			for (int i = 0; i < p; i++) {
				recv_cnt[i] = (y * (i + 1) / p - y * i / p)*x * 3;
				if (first) first = false;
				else recv_disp[i] = recv_disp[i - 1] + recv_cnt[i - 1];
			}
		}
		//std::cout << "process " << id << " finish " << endl;
		MPI_Gatherv(pdata, size*x * 3, MPI_CHAR, allData, recv_cnt, recv_disp, MPI_CHAR, 0, MPI_COMM_WORLD);
		//std::cout << " finish " << endl;

		if (!id) {
			//save single pic
			std::stringstream sio;
			Snapshot(allData, x, y, (std::string("./test/test.bmp").c_str()));
		}
	}
#endif
	MPI_Finalize();
}
#endif