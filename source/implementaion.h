#ifndef IMPLEMENTAION_HPP
#define IMPLEMENTAION_HPP

#include "interface.hpp"
#include "simple.hpp"

class movable_bounds : public i_movable_bounds<int2>
{

	protected:
	range2D bounds;

	public:
	movable_bounds(range2D bounds);

	int2 lower() const override;
	int2 upper() const override;
	movable_bounds& operator+=(const int2& offset) override;

};


#endif /* end of include guard */
