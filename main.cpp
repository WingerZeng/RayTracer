#include "scene.h"
using namespace std;
#include "mpi.h"
const int px = 1280;
const int py = 720;
//const int px = 720;
//const int py = 480;
//const int px = 1920;
//const int py = 1080;
//const int px = 5120;
//const int py = 2880;
#include "bmpgenerator.h"
#include "types.h"
int main(int argc,char*argv[]) {
	//初始化MPI
	MPI_Init(&argc, &argv);
	int id, p;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	//设置场景
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
	msmat_blur->setMirrorBlur(0.05);
	std::shared_ptr<Material> transMat(new Material(Color(0.10, 0.05, 0.07, 1), 1.5));
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

	int x, y;
	scene.getPixels(&x, &y);
	int begin = y * id / p;
	int end = y * (id + 1) / p;
	int size = end - begin;
	BYTE* pdata = new BYTE[size * x * 3];
	cout << begin << ' ' << end << endl;
	for (int i = begin, count = 0; i < end; i++, count++) {
		if (count % 20 == 0) {
			cout << "process " << id << " finish " << count << endl;
		}
		for (int j = 0; j < x; j++) {
			scene.run(j,i);
			Color color;
			scene.queryColor(j,i,color);
			pdata[count*x * 3 + j * 3 + 0] = int(color.x_ * 255);
			pdata[count*x * 3 + j * 3 + 1] = int(color.y_ * 255);
			pdata[count*x * 3 + j * 3 + 2] = int(color.z_ * 255);
			//cout << size * x * 3 << ' ' << count * x * 3 + j * 3 + 2 << std::endl;
		}
	}
	cout << "process " << id << " finish "<< endl;
	BYTE* allData;
	int* recv_cnt ;
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
			else recv_disp[i] = recv_disp[i - 1] + recv_cnt[i];
		}
	}
	cout << "gather1" << endl;
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gatherv(pdata, size*x*3, MPI_CHAR, allData, recv_cnt, recv_disp, MPI_CHAR, 0, MPI_COMM_WORLD);
	cout << "gather2" << endl;
	
	if (!id) {
		Snapshot(allData, x, y, "./test.bmp");
	}

	//scene.run();
	//scene.saveToBpm("./test.bmp");

	MPI_Finalize();
}
