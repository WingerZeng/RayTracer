#include "AviSaver.h"
#include <iostream>
using namespace cv;
//void AviSaver::AddBmp(std::string path)
//{
//}

void AviSaver::AddImg(const std::vector<uchar>& rgbs)
{
	//create a image matrix
	Mat img(height_, width_, CV_8UC3);
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			//set color for every pixels
			int index = 3 * width_*(height_ - 1 - i) + 3 * j;
			img.at<Vec3b>(i,j) = Vec3b(rgbs[index + 0], rgbs[index + 1], rgbs[index + 2]); //y axis in opencv is inverted
		}
	}
	//set image matrix as next frame
	if(loop_==1) *movie_ << img;
	else imags.push_back(img);
}

void AviSaver::AddImg(uchar * pdata)
{
	//create a image matrix
	Mat img(height_, width_, CV_8UC3);
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			//set color for every pixels
			int index = 3 * width_*(height_ - 1 - i) + 3 * j;
			img.at<Vec3b>(i, j) = Vec3b(pdata[index + 0], pdata[index + 1], pdata[index + 2]); //y axis in opencv is inverted
		}
	}
	//set image matrix as next frame
	if (loop_ == 1) *movie_ << img;
	else imags.push_back(img);
}

void AviSaver::Save() {
	if (loop_ >= 1) {
		for (int i = 0; i < loop_; i++) {
			std::cout << "loop" << i << std::endl;
			for (const auto& img : imags) {
				*movie_ << img;
			}
		}
	}
}


AviSaver::AviSaver()
{
	movie_.reset(new cv::VideoWriter());
}

void AviSaver::Open(std::string path, int width, int height, double fps, unsigned int loop)
{
	//movie_->open(path, cv::VideoWriter::fourcc('X', '2', '6', '4'), fps, cv::Size(width, height));
	movie_->open(path, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height));
	width_ = width;
	height_ = height;
	loop_ = loop;
}

bool AviSaver::isOpen()
{
	return movie_->isOpened();
}


AviSaver::~AviSaver()
{
}
