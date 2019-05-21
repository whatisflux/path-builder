#pragma once

#include <string>

class Waypoint
{
private:
	std::string serializeFloat(float v);
public:
	Waypoint();
	~Waypoint();

	float x;
	float y;
	bool insideIsLeft;

	std::string serialize();
};

