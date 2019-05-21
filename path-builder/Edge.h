#pragma once

#include <vector>
#include <string>
#include "Waypoint.h"

class Edge
{
public:
	Edge();
	~Edge();

	std::vector<Waypoint> waypoints;
	bool isClosed;

	std::string serialize();
};

