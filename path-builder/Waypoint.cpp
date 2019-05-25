#include "Waypoint.h"



Waypoint::Waypoint()
{
}

Waypoint::Waypoint(cv::Point2f pos, bool insideIsLeft)
{
	x = pos.x;
	y = pos.y;
	this->insideIsLeft = insideIsLeft;
}


Waypoint::~Waypoint()
{
}

std::string Waypoint::serializeFloat(float v)
{
	// Convert pointer to the float to a pointer to an int, and then deref it to get a value
	unsigned bits = *(unsigned*)&v;
	std::string serialized;
	for (int byte = 0; byte < 4; byte++)
	{
		// Shift the wanted bits to the lowest 8 bits
		// Bitwise-and it with 255 to clear out unwanted higher bits
		unsigned val = (bits >> (8 * (3 - byte))) & 0b11111111;
		serialized += (unsigned char)val;
	}
	return serialized;
}

std::string Waypoint::serialize()
{
	auto serializedX = serializeFloat(x);
	auto serializedY = serializeFloat(y);
	char serializedDir = insideIsLeft ? (char)0b11111111 : (char)0;

	return "p" + serializedX + serializedY + serializedDir;
}