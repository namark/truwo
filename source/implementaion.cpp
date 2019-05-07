#include "implementaion.h"

movable_bounds::movable_bounds(range2D bounds) : bounds(bounds) { }

int2 movable_bounds::lower() const
{
	return bounds.lower();
}

int2 movable_bounds::upper() const
{
	return bounds.upper();
}

movable_bounds& movable_bounds::operator+=(const int2& offset)
{
	bounds += offset;
	return *this;
}
