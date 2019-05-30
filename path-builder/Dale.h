#pragma once

#include <opencv2\opencv.hpp>
#include <vector>

using namespace cv;

class Dale
{
public:
	Dale();
	~Dale();

	void processFrame(Mat frame, Mat &hueOut, Mat &satOut, Mat &valOut, Mat &threshOut);
};

