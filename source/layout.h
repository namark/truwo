#ifndef LAYOUT_H
#define LAYOUT_H

#include <vector>
#include <functional>

#include "implementation.h"

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
constexpr void layout_bounds(ForwardItr begin, ForwardItr end, const int2& spacing)
{
	assert(begin != end);
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

template <typename Range>
constexpr void layout_bounds(Range range, const int2& spacing)
{
	using std::begin;
	using std::end;
	layout_bounds(begin(range), end(range), spacing);
}

class bounds_layout : public movable_bounds
{
	public:
	using container = std::vector<i_movable_bounds<int2>*>;
	container elements;
	int2 spacing;

	bounds_layout(container elements, int2 spacing = int2::zero()) :
		movable_bounds(invalid_range),
		elements(elements),
		spacing(spacing)
	{ }

	bounds_layout(int2 spacing) :
		movable_bounds(invalid_range),
		elements{},
		spacing(spacing)
	{ }

	bounds_layout& operator+=(const int2&) override;

	range2D update();
};

#endif /* end of include guard */


