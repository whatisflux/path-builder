#include "Dale.h"



Dale::Dale()
{
}


Dale::~Dale()
{
}

void Dale::processFrame(Mat frame, Mat &hueOut, Mat &satOut, Mat &valOut, Mat &threshOut)
{
	// Do your processing and output each channel to the matching argument
	std::vector<Mat> channels;
	split(frame, channels);
	hueOut = channels[0];
	satOut = channels[1];
	valOut = channels[2];
	
	int h_thresh_value = 12 * 1.7;
	int s_thresh_value = 21 * 2.54;
	int v_thresh_value = 37 * 2.54;
	int max_binary_value = 255;

	blur(frame, frame, Size(1, 7));
	inRange(frame, Scalar(h_thresh_value, s_thresh_value, v_thresh_value),
		Scalar(h_thresh_value + 18, max_binary_value, max_binary_value), threshOut); ;
}