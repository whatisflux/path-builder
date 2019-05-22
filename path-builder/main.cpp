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

	int outWidth = 800;
	int outHeight = outWidth * (outUHeight / outUWidth);



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

	// Perspective warp completed

	const int nwindows = 30;
	int windowHeight = outHeight / nwindows;
	int margin = windowHeight;

	// Begin sliding window search
	int leftx = outWidth / 2 - margin - 5;
	int rightx = outWidth / 2 + margin + 5;

	// Center of each window
	vector<Point2f> leftLaneComplex(0);
	vector<Point2f> rightLaneComplex(0);

	Mat slidingImg(flippedWarped);
#ifdef _DEBUG
	Mat slidImgDeb = slidingImg.clone();
#endif

#ifdef _DEBUG
	printf("Img dimension: (%d, %d)\n", outWidth, outHeight);
#endif
	for (int n = 0; n < nwindows; n++)
	{
		int winLow = outHeight - (n + 1)*windowHeight;
		int winHigh = outHeight - (n)*windowHeight - 1;
		int winLeftLeft = leftx - margin;
		int winLeftRight = leftx + margin;
		int winRightLeft = rightx - margin;
		int winRightRight = rightx + margin;
		

#ifdef _DEBUG
		printf("Left: (%d, %d)-(%d, %d)\t Right:(%d, %d)-(%d, %d)\n", winLeftLeft, winLow, winLeftRight, winHigh, winRightLeft, winLow, winRightRight, winHigh);

		rectangle(slidImgDeb, Rect(winLeftLeft, winLow, margin*2, windowHeight), Scalar(100, 100, 255), 1);
		rectangle(slidImgDeb, Rect(winRightLeft, winLow, margin*2, windowHeight), Scalar(255, 100, 100), 1);
#endif

		std::vector<Point2i> leftPoints(0);
		std::vector<Point2i> rightPoints(0);
		int leftsum = 0;
		int rightsum = 0;

		for (int y = winLow; y <= winHigh; y++)
		{
			for (int x = winLeftLeft; x < winLeftRight; x++)
			{
				if (x >= 0 && x < outWidth && slidingImg.at<Vec3b>(y, x)[0] > 0)
				{
					leftPoints.push_back(Point2i(x, y));
					leftsum += x;
				}
			}
			for (int x = winRightLeft; x < winRightRight; x++)
			{
				if (x >= 0 && x <= outWidth && slidingImg.at<Vec3b>(y, x)[0] > 0)
				{
					rightPoints.push_back(Point2i(x, y));
					rightsum += x;
				}
			}
		}

		float yCenter = (winLow + winHigh) / 2;

		// To calculate next leftx, use a best fit line of the points
		// in this window to estimate where the points in the
		// next window will be

		if (leftPoints.size() > 0)
		{
			leftLaneComplex.push_back(Point2f(leftx, yCenter));

			vector<float> leftLine(4);
			fitLine(leftPoints, leftLine, DIST_L2, 0, 0.01, 0.01);

			float u = ((winLow + winHigh) / 2 - windowHeight - leftLine[3]) / leftLine[1];
#ifdef _DEBUG
			line(slidImgDeb, Point2f(leftLine[2], leftLine[3]), Point2f(leftLine[2] + leftLine[0] * u, leftLine[3] + leftLine[1] * u), Scalar(0, 255, 0));
#endif
			// y1 = y0 + uv_y
			// x1 = x0 + uv_x
			// y0, x0, v_y, v_x, and y1 are known
			// u = (y1 - y0) / v_y
			// x1 = x0 + (y1 - y0) * v_x / v_y

			leftx = (int)(leftLine[2] + u * leftLine[0]);
			leftx = (leftx + leftsum / leftPoints.size()) / 2;
		}
		if (rightPoints.size() > 0)
		{
			rightLaneComplex.push_back(Point2f(rightx, yCenter));

			vector<float> rightLine(4);
			fitLine(rightPoints, rightLine, DIST_L2, 0, 0.01, 0.01);
			float u = ((winLow + winHigh) / 2 - windowHeight - rightLine[3]) / rightLine[1];
#ifdef _DEBUG
			line(slidImgDeb, Point2f(rightLine[2], rightLine[3]), Point2f(rightLine[2] + rightLine[0] * u, rightLine[3] + rightLine[1] * u), Scalar(0, 255, 0));
#endif
			rightx = (int)(rightLine[2] + u * rightLine[0]);
			rightx = (rightx + rightsum / rightPoints.size()) / 2;
		}

		if (rightPoints.size() == 0 || leftPoints.size() == 0) break;
	}

#ifdef _DEBUG
	imshow("Sliding debug", slidImgDeb);

	Mat lanesDebug(Size(outWidth, outHeight), CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < leftLaneComplex.size(); i++)
	{
		line(lanesDebug, leftLaneComplex[i - 1], leftLaneComplex[i], Scalar(0, 0, 255));
	}
	for (int i = 1; i < rightLaneComplex.size(); i++)
	{
		line(lanesDebug, rightLaneComplex[i - 1], rightLaneComplex[i], Scalar(255, 0, 0));
	}

	imshow("Lanes Path", lanesDebug);
#endif
		

	return Path();
}