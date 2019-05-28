#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <opencv2/opencv.hpp>
#include "Path.h"
#include "Camera.h"

#define IN_WIDTH 80
#define IN_HEIGHT 60
#define MAX_SCAN_HEIGHT 40
#define SCAN_HEIGHT 8
#define WALK_SIZE 5

#define OUT_WIDTH 200
#define HISTOGRAM_SCAN_HEIGHT 20
#define N_WINDOWS 20
#define WINDOW_WIDTH 10
#define MIN_WINDOW_POINTS 5

#define DILATE_SIZE 3
#define ERODE_SIZE 3

#define LINE_FIT_WEIGHT 0
#define AVERAGE_WEIGHT 1

#define SRC_IP "192.168.49.133"
#define DST_IP "192.168.49.1"

#ifdef _DEBUG
#define _D_DEBUG
#endif

using namespace cv;

// Transform edge from screen space to world space
Edge transformEdge(Edge edge, Size imgSize)
{
	Edge edgeT;
	edgeT.isClosed = edge.isClosed;
	for (int i = 0; i < edge.waypoints.size(); i++)
	{
		Waypoint o = edge.waypoints[i];
		Waypoint t;
		// Apply experimentally determined camera curve fit
		t.x = 4 * 1400 / (o.y + 7.5) * (o.x - imgSize.width / 2) / imgSize.width;
		t.y = 190000 / ((o.y + 16.9)*(o.y + 16.9));
		edgeT.waypoints.push_back(t);
	}
	return edgeT;
}
struct sockaddr_in createSockAddr(const char* ip, int port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	ULONG* addrUlong = new ULONG;
	inet_pton(AF_INET, ip, addrUlong);
	addr.sin_addr.s_addr = *addrUlong;

	return addr;
}

std::vector<int> generateHistogram(Mat image, int rowStart, int rowEnd);
int findPeakCol(std::vector<int> histogram, int colStart, int colEnd);
Path CreatePathFromBitmap(bool* bitmap, int width, int height);
Path WindowSlide(Mat image);
bool* CreateBitmapFromMat(Mat image);

int main_udp();
int main_test();

int main()
{
	return main_udp();
}

int main_udp()
{
	// Setup socket
	WSADATA wsaData;
	int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (status != NO_ERROR)
	{
		printf("ERROR STARTING WINSOCK: %d\n", status);
	}

	struct sockaddr_in local = createSockAddr(SRC_IP, 1234);
	struct sockaddr_in from;
	int fromlen = sizeof(from);

	struct sockaddr_in dest = createSockAddr(DST_IP, 1235);

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

	while (true)
	{
		char buffer[4096];
		ZeroMemory(buffer, sizeof(buffer));

		int bytes = recvfrom(socketS, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen);
		printf("Message received, checking for error\n");
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
				Path screenPath = CreatePathFromBitmap(bitmap, IN_WIDTH, IN_HEIGHT);

#ifdef _DEBUG

				Mat debSP(Size(IN_WIDTH, IN_HEIGHT), CV_8UC3, Scalar(0, 0, 0));
				for (int i = 1; i < screenPath.left.waypoints.size(); i++)
				{
					Waypoint p1 = screenPath.left.waypoints[i - 1];
					Waypoint p2 = screenPath.left.waypoints[i];
					line(debSP, Point(p1.x, p1.y), Point(p2.x, p2.y), Scalar(255, 255, 255));
				}
				imshow("Epic swag", debSP);
#endif

				// Path that is actually in floor space
				Path floorPath;
				floorPath.left = transformEdge(screenPath.left, Size(IN_WIDTH, IN_HEIGHT));
				floorPath.right = transformEdge(screenPath.right, Size(IN_WIDTH, IN_HEIGHT));

				std::string serializedFloorPath = floorPath.serialize();
				sendto(socketS, serializedFloorPath.c_str(), (int)serializedFloorPath.length(), 0, (sockaddr*)&dest, sizeof(dest));

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

	Camera cam(3.6, 5.03f, 2.83f);
	cam.height = 2.f;
	cam.theta = 0.43633231299f;
	cam.phi = 0;

	bool* bitmap = CreateBitmapFromMat(image);
	Rect2f realRect;
	Size imgSize;
	CreatePathFromBitmap(bitmap, image.cols, image.rows);


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

Path CreatePathFromBitmap(bool* bitmap, int width, int height)
{
	Mat image(height, width, CV_8UC3, Scalar(0, 0, 0));
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			uchar on = bitmap[y*width + x] * 255;
			image.at<Vec3b>(y, x)[0] = on;
			image.at<Vec3b>(y, x)[1] = on;
			image.at<Vec3b>(y, x)[2] = on;
		}
	}

#if ERODE_SIZE > 1
	Mat erodeKernel = getStructuringElement(MORPH_ELLIPSE, Size(ERODE_SIZE, ERODE_SIZE), Point(-1, -1));
	erode(image, image, erodeKernel, Point(-1, -1));
#endif
#if DILATE_SIZE > 1
	Mat dilateKernel = getStructuringElement(MORPH_ELLIPSE, Size(DILATE_SIZE, DILATE_SIZE), Point(-1, -1));
	dilate(image, image, dilateKernel, Point(-1, -1));
#endif
#if DILATE_SIZE > 1
	//Mat dilateKernel = getStructuringElement(MORPH_ELLIPSE, Size(DILATE_SIZE, DILATE_SIZE), Point(-1, -1));
	dilate(image, image, dilateKernel, Point(-1, -1));
#endif
#if ERODE_SIZE > 1
	//Mat erodeKernel = getStructuringElement(MORPH_ELLIPSE, Size(ERODE_SIZE, ERODE_SIZE), Point(-1, -1));
	erode(image, image, erodeKernel, Point(-1, -1));
#endif
	
	std::vector<Waypoint> waypoints;
	int winHeight = SCAN_HEIGHT;
	for (int y = height-1; y >= height - MAX_SCAN_HEIGHT; y -= winHeight)
	{
		line(image, Point(0, y), Point(width, y), Scalar(255, 0, 255));
		int xsum = 0;
		int xcount = 0;
		for (int h = y; h >= y - winHeight; h--)
		{
			int lastFound = -WALK_SIZE;
			for (int x = 0; x < width; x++)
			{
				if (x - lastFound < WALK_SIZE) continue;
				if (bitmap[h*width + x])
				{
					lastFound = x;
					xsum += x;
					xcount++;
					image.at<Vec3b>(Point(x, h)) = Vec3b(0, 255, 255);
				}
			}
		}
		// If not enough pixels, its gonna be too noisy
		if (xcount < MIN_WINDOW_POINTS) continue;
		float x = (float)xsum / (float)xcount;
		float y2 = y - winHeight / 2;
		Waypoint waypoint;
		waypoint.x = x;
		waypoint.y = y2;
		waypoints.push_back(waypoint);
	}
	Path path;
	Edge left;
	left.waypoints = waypoints;
	Edge right;
	right.waypoints = waypoints;
	path.left = left;
	path.right = right;

#ifdef _D_DEBUG
	imshow("Input", image);
#endif

	return path;

	//Path path = WindowSlide(image);

	return path;
}

std::vector<int> generateHistogram(Mat image, int rowStart, int rowEnd)
{
	std::vector<int> histogram(image.cols);
	if (rowStart < 0 || rowStart > image.rows) return histogram;
	if (rowEnd < 0 || rowStart > image.rows || rowEnd <= rowStart) return histogram;

	for (int x = 0; x < image.cols; x++)
	{
		int sum = 0;
		for (int y = rowStart; y < rowEnd; y++)
		{
			sum += (int)image.at<Vec3b>(y, x)[0];
		}
		histogram[x] = sum;
	}
	return histogram;
}

int findPeakCol(std::vector<int> histogram, int colStart, int colEnd)
{
	if (colStart < 0 || colStart > histogram.size()) return -1;
	if (colEnd < 0 || colEnd > histogram.size() || colEnd <= colStart) return -1;

	int max = -1000000000;
	int maxIndex = -1;
	for (int i = colStart; i < colEnd; i++)
	{
		if (histogram[i] > max)
		{
			max = histogram[i];
			maxIndex = i;
		}
	}
	return maxIndex;
}

Path WindowSlide(Mat image)
{
	int height = image.rows;
	int width = image.cols;

	int windowHeight = height / N_WINDOWS;
	int margin = WINDOW_WIDTH / 2;

	// Begin sliding window search
	auto laneHistogram = generateHistogram(image, height - HISTOGRAM_SCAN_HEIGHT, height);
	int leftx = findPeakCol(laneHistogram, 0, width / 2);
	int rightx = findPeakCol(laneHistogram, width / 2, width);


	// Center of each window
	std::vector<Point2f> leftLaneComplex(0);
	std::vector<Point2f> rightLaneComplex(0);

	Mat slidingImg(image);
#ifdef _D_DEBUG
	Mat slidImgDeb = slidingImg.clone();

	line(slidImgDeb, Point(width / 2, height - HISTOGRAM_SCAN_HEIGHT), Point(width / 2, height), Scalar(255, 0, 255));
	line(slidImgDeb, Point(0, height - HISTOGRAM_SCAN_HEIGHT), Point(width - 1, height - HISTOGRAM_SCAN_HEIGHT), Scalar(255, 0, 255));
#endif

	// Keep track whether any windows have contained points yet
	bool leftContainedPoints = false;
	bool rightContainedPoints = false;

#ifdef _D_DEBUG
	//printf("Img dimension: (%d, %d)\n", OUT_WIDTH, outHeight);
#endif
	for (int n = 0; n < N_WINDOWS; n++)
	{
		int winLow = height - (n + 1)*windowHeight;
		int winHigh = height - (n)*windowHeight - 1;
		int winLeftLeft = leftx - margin;
		int winLeftRight = leftx + margin;
		int winRightLeft = rightx - margin;
		int winRightRight = rightx + margin;


#ifdef _D_DEBUG
		//printf("Left: (%d, %d)-(%d, %d)\t Right:(%d, %d)-(%d, %d)\n", winLeftLeft, winLow, winLeftRight, winHigh, winRightLeft, winLow, winRightRight, winHigh);

		rectangle(slidImgDeb, Rect(winLeftLeft, winLow, margin * 2, windowHeight), Scalar(100, 100, 255), 1);
		rectangle(slidImgDeb, Rect(winRightLeft, winLow, margin * 2, windowHeight), Scalar(255, 100, 100), 1);
#endif

		std::vector<Point2i> leftPoints(0);
		std::vector<Point2i> rightPoints(0);
		int leftsum = 0;
		int rightsum = 0;

		for (int y = winLow; y <= winHigh; y++)
		{
			for (int x = winLeftLeft; x < winLeftRight; x++)
			{
				if (x >= 0 && x < width && slidingImg.at<Vec3b>(y, x)[0] > 0)
				{
					leftPoints.push_back(Point2i(x, y));
					leftsum += x;
				}
			}
			for (int x = winRightLeft; x < winRightRight; x++)
			{
				if (x >= 0 && x < width && slidingImg.at<Vec3b>(y, x)[0] > 0)
				{
					rightPoints.push_back(Point2i(x, y));
					rightsum += x;
				}
			}
		}

		float yCenter = (winLow + winHigh) / 2.f;

		// To calculate next leftx, use a best fit line of the points
		// in this window to estimate where the points in the
		// next window will be

		// First break out if things are bad
		bool rightIsBad = rightPoints.size() < MIN_WINDOW_POINTS && rightContainedPoints;
		bool leftIsBad = leftPoints.size() < MIN_WINDOW_POINTS && leftContainedPoints;
		if (rightIsBad || leftIsBad) break;
		if (rightPoints.size() >= MIN_WINDOW_POINTS) rightContainedPoints = true;
		if (leftPoints.size() >= MIN_WINDOW_POINTS) leftContainedPoints = true;

		leftLaneComplex.push_back(Point2f((float)leftx, yCenter));
		rightLaneComplex.push_back(Point2f((float)rightx, yCenter));
		if (leftPoints.size() >= MIN_WINDOW_POINTS)
		{


			std::vector<float> leftLine(4);
			fitLine(leftPoints, leftLine, DIST_L2, 0, 0.01, 0.01);

			float u = ((winLow + winHigh) / 2 - windowHeight - leftLine[3]) / leftLine[1];
#ifdef _D_DEBUG
			line(slidImgDeb, Point2f(leftLine[2], leftLine[3]), Point2f(leftLine[2] + leftLine[0] * u, leftLine[3] + leftLine[1] * u), Scalar(0, 255, 0));
#endif

			leftx = (int)(leftLine[2] + u * leftLine[0]);
			leftx = leftx * LINE_FIT_WEIGHT + leftsum / (int)leftPoints.size() * AVERAGE_WEIGHT;
		}
		if (rightPoints.size() >= MIN_WINDOW_POINTS)
		{

			std::vector<float> rightLine(4);
			fitLine(rightPoints, rightLine, DIST_L2, 0, 0.01, 0.01);
			float u = ((winLow + winHigh) / 2 - windowHeight - rightLine[3]) / rightLine[1];
#ifdef _D_DEBUG
			line(slidImgDeb, Point2f(rightLine[2], rightLine[3]), Point2f(rightLine[2] + rightLine[0] * u, rightLine[3] + rightLine[1] * u), Scalar(0, 255, 0));
#endif
			rightx = (int)(rightLine[2] + u * rightLine[0]);
			rightx = rightx * LINE_FIT_WEIGHT + rightsum / (int)rightPoints.size() * AVERAGE_WEIGHT;
		}
	}

#ifdef _D_DEBUG
	imshow("Sliding debug", slidImgDeb);

	Mat lanesDebug(Size(width, height), CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < leftLaneComplex.size(); i++)
	{
		line(lanesDebug, leftLaneComplex[i - 1], leftLaneComplex[i], Scalar(0, 0, 255), 2);
	}
	for (int i = 1; i < rightLaneComplex.size(); i++)
	{
		line(lanesDebug, rightLaneComplex[i - 1], rightLaneComplex[i], Scalar(255, 0, 0), 2);
	}

	imshow("Lanes Path", lanesDebug);
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
	path.left = left;
	path.right = right;

	return path;
}