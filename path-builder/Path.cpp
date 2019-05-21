#include "Path.h"



Path::Path()
{
}


Path::~Path()
{
}

std::string Path::serialize()
{
	return left.serialize() + right.serialize();
}