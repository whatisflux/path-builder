#include "Edge.h"



Edge::Edge()
{
	waypoints = std::vector<Waypoint>(0);
}


Edge::~Edge()
{
}

std::string Edge::serialize()
{
	std::string serialized;
	serialized += "e";

	for (int i = 0; i < waypoints.size(); i++)
	{
		serialized += waypoints[i].serialize();
	}

	if (isClosed) serialized += "z";

	return serialized;
}
