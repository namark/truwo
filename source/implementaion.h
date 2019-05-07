#ifndef IMPLEMENTAION_HPP
#define IMPLEMENTAION_HPP

#include "interface.hpp"
#include "simple.hpp"

class movable_bounds : public i_movable_bounds<int2>
{
	public:
	movable_bounds(range2D bounds);

	int2 lower() const override;
	int2 upper() const override;
	movable_bounds& operator+=(const int2& offset) override;

	virtual ~movable_bounds() = default;

	protected:
	movable_bounds(const movable_bounds&) = default;
	movable_bounds& operator=(const movable_bounds&) = default;
	range2D bounds;

};


#endif /* end of include guard */
