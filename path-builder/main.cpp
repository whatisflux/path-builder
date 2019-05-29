#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <opencv2/opencv.hpp>
#include "Craig.h"

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
				Mat image(Size(IN_WIDTH, IN_HEIGHT), CV_8UC1, Scalar(0, 0, 0));

				for (int i = 0; i < bytes; i++)
				{
					for (int bit = 0; bit < 8; bit++)
					{
						// Get the bit-th bit of the byte at buffer[i]
						unsigned char val = (buffer[i] >> bit) & 1;
						int p = i * 8 + bit;
						int y = p / IN_WIDTH;
						int x = p % IN_WIDTH;
						image.at<uchar>(Point(x, y)) = val * 255;
					}
				}

				Craig c;
				auto path = c.processImage(image);

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
	image = imread("resources/test-2.png", IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		std::cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	cvtColor(image, image, COLOR_RGB2GRAY);

	resize(image, image, Size(160, 120), 0, 0);

	Craig c;
	auto path = c.processImage(image);
	imshow("Input", image);

	waitKey(0); // Wait for a keystroke in the window

	return 0;
}