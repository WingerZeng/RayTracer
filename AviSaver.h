#pragma once
#include <opencv/cv.h>
#include <opencv2/videoio.hpp>
#include <string>
#include <vector>
class AviSaver
{
public:
	//void AddBmp(std::string path);
	void AddImg(const std::vector<uchar>& rgbs);
	void AddImg(uchar* pdata);
	void Save();;
	AviSaver();
	void Open(std::string path, int width, int height, double fps, unsigned int loop = 1);
	bool isOpen();
	~AviSaver();

private:
	std::shared_ptr<cv::VideoWriter> movie_;
	int width_;
	int height_;
	int loop_;
	std::vector<cv::Mat> imags; //for loop
};

