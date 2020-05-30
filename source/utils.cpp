#include <type_traits>
#include "utils.hpp"

template <typename Filler, typename Range>
void outline(Range bounds, const Filler& fill)
{
	constexpr size_t dimensions = std::remove_reference_t<decltype(bounds.lower())>::dimensions;
	for(size_t coord = 0; coord < dimensions; ++coord)
	{
		auto lower_edge = bounds;
		lower_edge.upper()[coord] = lower_edge.lower()[coord]+1;
		auto upper_edge = bounds;
		upper_edge.lower()[coord] = upper_edge.upper()[coord]-1;

		std::invoke(fill, lower_edge);
		std::invoke(fill, upper_edge);
	}
}

void outline(const graphical::surface& target, graphical::color color, range2D bounds)
{
	outline(bounds, [&](range2D bounds){ graphical::fill(target, color, bounds); });
}

void lowlight(const graphical::surface& target, graphical::color color, range2D bounds, int2 dim)
{
	range2D target_bounds = rect{target.size()};
	bounds = intersection(bounds, target_bounds);
	if(!bounds.valid())
		return; // we are off target

	// unless there is no distance to travel, make sure to step forward
	assert(to_conjunction( (bounds.upper() - bounds.lower() == int2::zero()) | (dim > int2::zero()) ));

	std::visit([&](auto writer)
	{
		auto i = bounds.lower();
		auto dimension = i.begin();
		while(dimension != i.end())
		{
			writer.set(color, i);
			dimension = support::advance_vector(i, bounds.lower(), bounds.upper(), dim);
		}
	}, target.pixels());
}

void outline_lowlight(const graphical::surface& target, graphical::color color, range2D bounds, int2 dim)
{
	outline(bounds, [&](range2D bounds){ lowlight(target, color, bounds, dim); });
}

