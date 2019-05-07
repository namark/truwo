#ifndef LAYOUT_H
#define LAYOUT_H

#include <vector>
#include <functional>

#include "implementaion.h"

inline int2 signum(int2 v) noexcept;

template <typename Range>
int2 select_corner(const Range& rng, int2 direction) noexcept
{
	// center + direction * half_size
	return
	(
		(rng.lower() + rng.upper()) // center * 2
		+ direction * (rng.upper() - rng.lower()) // direction * size
	) / 2; // all halved
}

template <typename ForwardItr>
void layout_bounds(ForwardItr begin, ForwardItr end, const int2& spacing) noexcept
{
	const auto mask = int2(spacing != int2::zero());
	const auto corner = signum(spacing);
	for(auto prev = begin++; begin != end; ++begin, ++prev)
	{
		*begin +=
		(
			select_corner(*prev, corner)
			+ spacing
			- select_corner(*begin, -corner)
		) * mask;
	}
}

class bounds_layout : public movable_bounds
{
	public:
	static const range2D invalid_bounds;
	using container = std::vector<i_movable_bounds<int2>*> ;
	container elements;
	int2 spacing;

	bounds_layout(container elements, int2 spacing = int2::zero()) :
		movable_bounds(invalid_bounds),
		elements(elements),
		spacing(spacing)
	{ }

	bounds_layout(int2 spacing) :
		movable_bounds(invalid_bounds),
		elements{},
		spacing(spacing)
	{ }

	bounds_layout& operator+=(const int2&) override;

	range2D update();
};

#endif /* end of include guard */


