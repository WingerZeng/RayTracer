#pragma once
#include <cuda_runtime.h>  //Í·ÎÄ¼þ
#include <device_launch_parameters.h>
#include "cudaRayTracer.h"

//__global__ void test() {
//	printf("Hi Cuda World");
//}
//
//int main(int argc, char** argv)
//{
//	test << <1, 1 >> > ();
//	cudaDeviceSynchronize();
//	return 0;
//}


//__global__ void cudaRayColor(Scene* scene, std::vector<Ray>* ray, double t0, double t1, int jumptime, std::vector<Color>* color) {
//	(*color)[threadIdx.x] = scene->rayColor((*ray)[threadIdx.x], t0, t1, jumptime);
//}
//
//void getRayColor(Scene* scene, std::vector<Ray> ray, double t0, double t1, int jumptime, std::vector<Color>& color){
//	cudaRayColor<<<1,ray.size()>>>(scene,&ray,t0,t1,jumptime,&color);
//	cudaDeviceSynchronize();
//}
//
