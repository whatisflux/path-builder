#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

Camera::Camera(float focalLength, float screenWidth, float screenHeight)
{
	this->focalLength = focalLength;
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	updateFOV();
}

void Camera::updateFOV()
{
	fovx = 2 * atan(screenWidth / (2 * focalLength));
	fovy = 2 * atan(screenHeight / (2 * focalLength));
}

void Camera::setFocalLength(float focalLength)
{
	this->focalLength = focalLength;
	updateFOV();
}
void Camera::setScreenWidth(float screenWidth)
{
	this->screenWidth = screenWidth;
	updateFOV();
}
void Camera::setScreenHeight(float screenHeight)
{
	this->screenHeight = screenHeight;
	updateFOV();
}

cv::Vec2f Camera::ProjectPoint(int pixX, int pixY, int imgWidth, int imgHeight)
{
	float sy = (float)pixY / (float)imgHeight * 2.f - 1.f;
	float pixTheta = atan(screenHeight*sy / (2.f*focalLength));
	float theta = this->theta + pixTheta;

	if (theta <= 0 || theta >= CV_PI) return 0;

	float rayLength = height / tanf(theta);

	float sx = (float)pixX / (float)imgWidth * 2.f - 1.f;
	float pixPhi = atan(screenWidth*sx / (2.f*focalLength));
	float phi = pixPhi;

	float dx = rayLength * sin(phi);
	float dy = rayLength * cos(phi);

	return { dx, dy };
}