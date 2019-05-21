#pragma once

#include <string>
#include "Edge.h"

class Path
{
public:
	Path();
	~Path();

	Edge left;
	Edge right;

	std::string serialize();
};

