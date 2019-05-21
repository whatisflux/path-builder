#include <iostream>
#include <opencv2/opencv.hpp>
#include "Path.h"
#include "Camera.h"

using namespace cv;
using namespace std;

Path CreatePathFromBitmap(bool* bitmap, int width, int height, Camera cam);
bool* CreateBitmapFromMat(Mat image);

int main(int argc, char** argv)
{

	Mat image;
	image = imread("resources/rope-mat-1-s6-cam.png", IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Camera cam(4.89f, 5.95f, 3.35f);
	cam.height = 2.f;
	cam.theta = 0.43633231299f;
	cam.phi = 0;

	bool* bitmap = CreateBitmapFromMat(image);
	CreatePathFromBitmap(bitmap, image.cols, image.rows, cam);


	waitKey(0); // Wait for a keystroke in the window

	free(bitmap);

	return 0;
}

bool* CreateBitmapFromMat(Mat image)
{
	bool* bitmap = (bool*)malloc(sizeof(bool) * (image.rows * image.cols));

	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			uchar imgV = image.at<Vec3b>(y, x).val[0];
			bool val = imgV > 0 ? true : false;
			bitmap[y*image.cols + x] = val;
		}
	}

	return bitmap;
}

Path CreatePathFromBitmap(bool* bitmap, int width, int height, Camera cam)
{
	Mat image(height, width, CV_8UC3, Scalar(0, 0, 0));
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uchar on = bitmap[y*width + x]*255;
			image.at<Vec3b>(y, x)[0] = on;
			image.at<Vec3b>(y, x)[1] = on;
			image.at<Vec3b>(y, x)[2] = on;
		}
	}

	imshow("Input", image);

	// Performing the perspective warp

	Mat inputQuad(4, 2, CV_32F);
	Mat outputQuad(4, 2, CV_32F);

	Mat lambda(2, 4, CV_32FC1);
	Mat warped(500, 500, CV_8UC3);

	Point2f topLeft = cam.ProjectPoint(0, 0, width, height);
	Point2f topRight = cam.ProjectPoint(width - 1, 0, width, height);
	Point2f botRight = cam.ProjectPoint(width - 1, height - 1, width, height);
	Point2f botLeft = cam.ProjectPoint(0, height - 1, width, height);

	Point2f origin = Point2f(topLeft.x, botLeft.y);
	float outUWidth = topRight.x - topLeft.x;
	float outUHeight = topLeft.y - botLeft.y;
	topLeft -= origin;
	topRight -= origin;
	botRight -= origin;
	botLeft -= origin;

	int outWidth = 500;
	int outHeight = 500 * (outUHeight / outUWidth);



	inputQuad.at<float>(0, 0) = 0;
	inputQuad.at<float>(0, 1) = 0;

	inputQuad.at<float>(1, 0) = width - 1;
	inputQuad.at<float>(1, 1) = 0;

	inputQuad.at<float>(2, 0) = width - 1;
	inputQuad.at<float>(2, 1) = height - 1;

	inputQuad.at<float>(3, 0) = 0;
	inputQuad.at<float>(3, 1) = height - 1;

	outputQuad.at<float>(0, 0) = topLeft.x * outWidth / outUWidth;
	outputQuad.at<float>(0, 1) = topLeft.y * outHeight / outUHeight;

	outputQuad.at<float>(1, 0) = topRight.x * outWidth / outUWidth;
	outputQuad.at<float>(1, 1) = topRight.y * outHeight / outUHeight;

	outputQuad.at<float>(2, 0) = botRight.x * outWidth / outUWidth;
	outputQuad.at<float>(2, 1) = botRight.y * outHeight / outUHeight;

	outputQuad.at<float>(3, 0) = botLeft.x * outWidth / outUWidth;
	outputQuad.at<float>(3, 1) = botLeft.y * outHeight / outUHeight;



	lambda = getPerspectiveTransform(inputQuad, outputQuad);
	warpPerspective(image, warped, lambda, Size(outWidth, outHeight));

	Mat flippedWarped(warped.rows, warped.cols, CV_8UC3);
	flip(warped, flippedWarped, 0);

	imshow("Warped", flippedWarped);

	// Perspective warp completed

	return Path();
}