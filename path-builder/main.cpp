#include <iostream>
#include <winsock2.h>
#include <opencv2/opencv.hpp>
#include "Path.h"
#include "Camera.h"

#define IN_WIDTH 80
#define IN_HEIGHT 60

#define OUT_WIDTH 200
#define N_WINDOWS 30
#define WINDOW_WIDTH 10

#ifdef _DEBUG
#define _D_DEBUG
#endif

using namespace cv;

// Transform edge from screen space to world space
Edge transformEdge(Edge edge, Size src, Rect2f dst)
{
	Edge edgeT;
	edgeT.isClosed = edge.isClosed;
	for (int i = 0; i < edge.waypoints.size(); i++)
	{
		Waypoint o = edge.waypoints[i];
		Waypoint t;
		t.x = (o.x / src.width) * dst.width + dst.x;
		t.y = (o.y / src.height) * dst.height + dst.y;
		t.insideIsLeft = o.insideIsLeft;
		edgeT.waypoints.push_back(t);
	}
	return edgeT;
}

Path CreatePathFromBitmap(bool* bitmap, int width, int height, Camera cam, Rect2f* realRect, Size* imgSize);
bool* CreateBitmapFromMat(Mat image);

int main_udp();
int main_test();

int main()
{
	return main_test();
}

int main_udp()
{
	// Setup socket
	WSADATA wsaData;
	int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (status != 0)
	{
		printf("ERROR STARTING WINSOCK: %d\n", status);
	}

	struct sockaddr_in local;
	struct sockaddr_in from;
	int fromlen = sizeof(from);
	local.sin_family = AF_INET;
	local.sin_port = htons(1234);
	local.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	//dest.sin_addr.s_addr = inet_addr("192.168.43.99");
	dest.sin_port = htons(1234);

	SOCKET socketS = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketS == INVALID_SOCKET)
	{
		printf("ERROR MAKING SOCKET: %d", WSAGetLastError());
	}
	int result = bind(socketS, (sockaddr*)&local, sizeof(local));
	if (result == SOCKET_ERROR)
	{
		printf("ERROR BINDING SOCKET: %d", WSAGetLastError());
	}

	Camera cam(4.89f, 5.95f, 3.35f);
	cam.height = 2.f;
	cam.theta = 0.43633231299f;
	cam.phi = 0;
	while (true)
	{
		char buffer[4096];
		ZeroMemory(buffer, sizeof(buffer));

		int bytes = recvfrom(socketS, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen);
		printf("Message received, checking error\n");
		if (bytes != SOCKET_ERROR)
		{
			printf("Received message from %s (%d bytes)\n", "someone", bytes);
			if (bytes == IN_WIDTH * IN_HEIGHT / 8)
			{
				// Image was received
				bool bitmap[IN_WIDTH * IN_HEIGHT];

				for (int i = 0; i < bytes; i++)
				{
					for (int bit = 0; bit < 8; bit++)
					{
						// Get the bit-th bit of the byte at buffer[i]
						unsigned char val = (buffer[i] >> bit) & 1;
						int p = (i * 8 + bit) * 4;
						bitmap[i * 8 + bit] = val;
					}
				}

				Rect2f realRect;
				Size2i imgSize;
				Path screenPath = CreatePathFromBitmap(bitmap, IN_WIDTH, IN_HEIGHT, cam, &realRect, &imgSize);

				Mat dispPath(imgSize, CV_8UC3, Scalar(0, 0, 0));
				for (int i = 1; i < screenPath.left.waypoints.size(); i++)
				{
					Waypoint point1 = screenPath.left.waypoints[i - 1];
					Waypoint point2 = screenPath.left.waypoints[i];
					line(dispPath, Point2i(point1.x, point1.y), Point2i(point2.x, point2.y), Scalar(0, 0, 255));
				}
				for (int i = 1; i < screenPath.right.waypoints.size(); i++)
				{
					Waypoint point1 = screenPath.right.waypoints[i - 1];
					Waypoint point2 = screenPath.right.waypoints[i];
					line(dispPath, Point2i(point1.x, point1.y), Point2i(point2.x, point2.y), Scalar(255, 0, 0));
				}

				// Path that is actually in floor space
				Path floorPath;
				floorPath.left = transformEdge(screenPath.left, imgSize, realRect);
				floorPath.right = transformEdge(screenPath.right, imgSize, realRect);

				std::string serializedFloorPath = floorPath.serialize();
				sendto(socketS, serializedFloorPath.c_str(), serializedFloorPath.length(), 0, (sockaddr*)&dest, sizeof(dest));

				imshow("Path", dispPath);
				waitKey(1);
			}
		}
		else
		{
			printf("Error! WSA: %d\n", WSAGetLastError());
		}
	}

	return 0;
}

int main_test()
{

	Mat image;
	image = imread("resources/rope-mat-1-s6-cam.png", IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	Camera cam(4.89f, 5.95f, 3.35f);
	cam.height = 2.f;
	cam.theta = 0.43633231299f;
	cam.phi = 0;

	bool* bitmap = CreateBitmapFromMat(image);
	Rect2f realRect;
	Size imgSize;
	CreatePathFromBitmap(bitmap, image.cols, image.rows, cam, &realRect, &imgSize);


	waitKey(0); // Wait for a keystroke in the window

	free(bitmap);

	return 0;
}

void dumbDumb()
{
	
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

Path CreatePathFromBitmap(bool* bitmap, int width, int height, Camera cam, Rect2f* realRect, Size* imgSize)
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

#ifdef _D_DEBUG
	imshow("Input", image);
#endif

	// Performing the perspective warp

	Mat inputQuad(4, 2, CV_32F);
	Mat outputQuad(4, 2, CV_32F);

	Mat lambda(2, 4, CV_32FC1);
	Mat warped;

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

	int outHeight = OUT_WIDTH * (outUHeight / outUWidth);
	*imgSize = Size(OUT_WIDTH, outHeight);



	inputQuad.at<float>(0, 0) = 0;
	inputQuad.at<float>(0, 1) = 0;

	inputQuad.at<float>(1, 0) = width - 1;
	inputQuad.at<float>(1, 1) = 0;

	inputQuad.at<float>(2, 0) = width - 1;
	inputQuad.at<float>(2, 1) = height - 1;

	inputQuad.at<float>(3, 0) = 0;
	inputQuad.at<float>(3, 1) = height - 1;

	outputQuad.at<float>(0, 0) = topLeft.x * OUT_WIDTH / outUWidth;
	outputQuad.at<float>(0, 1) = topLeft.y * outHeight / outUHeight;

	outputQuad.at<float>(1, 0) = topRight.x * OUT_WIDTH / outUWidth;
	outputQuad.at<float>(1, 1) = topRight.y * outHeight / outUHeight;

	outputQuad.at<float>(2, 0) = botRight.x * OUT_WIDTH / outUWidth;
	outputQuad.at<float>(2, 1) = botRight.y * outHeight / outUHeight;

	outputQuad.at<float>(3, 0) = botLeft.x * OUT_WIDTH / outUWidth;
	outputQuad.at<float>(3, 1) = botLeft.y * outHeight / outUHeight;

	// Undo origin transform
	topLeft += origin;
	topRight += origin;
	botRight += origin;
	botLeft += origin;

	*realRect = Rect2f(topLeft, Size2f(outUWidth, outUHeight));


	lambda = getPerspectiveTransform(inputQuad, outputQuad);
	Mat lambdaInverse = getPerspectiveTransform(outputQuad, inputQuad);
	warpPerspective(image, warped, lambda, Size(OUT_WIDTH, outHeight));

	Mat flippedWarped(warped.rows, warped.cols, CV_8UC3);
	flip(warped, flippedWarped, 0);

	// Perspective warp completed

	int windowHeight = outHeight / N_WINDOWS;
	int margin = WINDOW_WIDTH / 2;

	// Begin sliding window search
	int leftx = OUT_WIDTH / 2 - margin - 1;
	int rightx = OUT_WIDTH / 2 + margin + 1;

	// Center of each window
	std::vector<Point2f> leftLaneComplex(0);
	std::vector<Point2f> rightLaneComplex(0);

	Mat slidingImg(flippedWarped);
#ifdef _D_DEBUG
	Mat slidImgDeb = slidingImg.clone();
#endif

#ifdef _D_DEBUG
	printf("Img dimension: (%d, %d)\n", OUT_WIDTH, outHeight);
#endif
	for (int n = 0; n < N_WINDOWS; n++)
	{
		int winLow = outHeight - (n + 1)*windowHeight;
		int winHigh = outHeight - (n)*windowHeight - 1;
		int winLeftLeft = leftx - margin;
		int winLeftRight = leftx + margin;
		int winRightLeft = rightx - margin;
		int winRightRight = rightx + margin;
		

#ifdef _D_DEBUG
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
				if (x >= 0 && x < OUT_WIDTH && slidingImg.at<Vec3b>(y, x)[0] > 0)
				{
					leftPoints.push_back(Point2i(x, y));
					leftsum += x;
				}
			}
			for (int x = winRightLeft; x < winRightRight; x++)
			{
				if (x >= 0 && x < OUT_WIDTH && slidingImg.at<Vec3b>(y, x)[0] > 0)
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

			std::vector<float> leftLine(4);
			fitLine(leftPoints, leftLine, DIST_L2, 0, 0.01, 0.01);

			float u = ((winLow + winHigh) / 2 - windowHeight - leftLine[3]) / leftLine[1];
#ifdef _D_DEBUG
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

			std::vector<float> rightLine(4);
			fitLine(rightPoints, rightLine, DIST_L2, 0, 0.01, 0.01);
			float u = ((winLow + winHigh) / 2 - windowHeight - rightLine[3]) / rightLine[1];
#ifdef _D_DEBUG
			line(slidImgDeb, Point2f(rightLine[2], rightLine[3]), Point2f(rightLine[2] + rightLine[0] * u, rightLine[3] + rightLine[1] * u), Scalar(0, 255, 0));
#endif
			rightx = (int)(rightLine[2] + u * rightLine[0]);
			rightx = (rightx + rightsum / rightPoints.size()) / 2;
		}

		if (rightPoints.size() == 0 || leftPoints.size() == 0) break;
	}

#ifdef _D_DEBUG
	imshow("Sliding debug", slidImgDeb);

	Mat lanesDebug(Size(OUT_WIDTH, outHeight), CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < leftLaneComplex.size(); i++)
	{
		line(lanesDebug, leftLaneComplex[i - 1], leftLaneComplex[i], Scalar(0, 0, 255));
	}
	for (int i = 1; i < rightLaneComplex.size(); i++)
	{
		line(lanesDebug, rightLaneComplex[i - 1], rightLaneComplex[i], Scalar(255, 0, 0));
	}

	imshow("Lanes Path", lanesDebug);

	Mat lanesFlippedDeb;
	Mat lanesCamDebug;
	flip(lanesDebug, lanesFlippedDeb, 0);
	warpPerspective(lanesFlippedDeb, lanesCamDebug, lambda, Size(width, height), WARP_INVERSE_MAP);
	imshow("Lanes Path Cam", lanesCamDebug);
#endif

	Path path;
	Edge left;
	Edge right;

	for (int i = 0; i < leftLaneComplex.size(); i++)
	{
		left.waypoints.push_back(Waypoint(leftLaneComplex[i], false));
	}
	for (int i = 0; i < rightLaneComplex.size(); i++)
	{
		right.waypoints.push_back(Waypoint(rightLaneComplex[i], false));
	}
		

	return path;
}