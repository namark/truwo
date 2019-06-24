#include "utils.hpp"

void outline(const graphical::surface& target, graphical::color color, range2D bounds)
{
	for(size_t coord = 0; coord < int2::dimensions; ++coord)
	{
		auto lower_edge = bounds;
		lower_edge.upper()[coord] = lower_edge.lower()[coord]+1;
		auto upper_edge = bounds;
		upper_edge.lower()[coord] = upper_edge.upper()[coord]-1;

		graphical::fill(target, color, lower_edge);
		graphical::fill(target, color, upper_edge);
	}
}

void highlight(const graphical::surface& target, graphical::color color, range2D bounds, int2 sparsity)
{
	range2D target_bounds = rect{target.size()};
	bounds = intersection(bounds, target_bounds);
	if(!bounds.valid())
		return;

	std::visit([&](auto writer)
	{
		int2 i = bounds.lower();
		auto dimension = i.begin();
		while(dimension != i.end())
		{
			writer.set(static_cast<uint32_t>((color)), i);
			dimension = support::advance_vector(i, bounds.lower(), bounds.upper(), sparsity);
		}
	}, target.pixels());
}

