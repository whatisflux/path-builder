#pragma once

#include <string>
#include <opencv2/core.hpp>

class Waypoint
{
private:
	std::string serializeFloat(float v);
public:
	Waypoint();
	~Waypoint();

	Waypoint(cv::Point2f position, bool insideIsLeft);

	float x;
	float y;
	bool insideIsLeft;

	std::string serialize();
};

