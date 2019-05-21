#pragma once

#include <math.h>
#include <opencv2/core.hpp>

class Camera
{
private:
	float fovx;
	float fovy;

	float focalLength;
	float screenWidth;
	float screenHeight;

	void updateFOV();
public:
	float theta;
	float phi;
	float height;

	Camera();
	~Camera();

	Camera(float focalLength, float screenWidth, float screenHeight);

	float getFovx() { return fovx; }
	float getFovy() { return fovy; }

	void setFocalLength(float focalLength);
	void setScreenWidth(float screenWidth);
	void setScreenHeight(float screenHeight);

	cv::Vec2f ProjectPoint(int pixX, int pixY, int imgWidth, int imgHeight);
};

