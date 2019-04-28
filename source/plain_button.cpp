#include "plain_button.h"

plain_button::plain_button(graphical::color color, range2D bounds) :
	button(bounds), color(color)
{ }

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

void highlight(const graphical::surface& target, graphical::color color, range2D bounds, int2 sparsity = int2(2,1))
{
	std::visit([&](auto writer)
	{
		bounds = support::intersection<int2>(bounds, rect{target.size()});
		int2 i = bounds.lower();
		for(; i < bounds.upper(); i.y() += sparsity.y())
		{
			for(; i < bounds.upper(); i.x() += sparsity.x())
			{
				writer.set(static_cast<uint32_t>((color)), i);
			}
			i.x() = bounds.lower().x() + (i.y()%sparsity.x());
		}
	}, target.pixels());
}

void plain_button::draw(const graphical::surface& target)
{
	switch(current)
	{
		case button::state::hover:
			highlight(target, color, bounds, int2(2,2));
		case button::state::idle:
			outline(target, color, bounds);
		break;

		case button::state::pressed:
			graphical::fill(target, color, bounds);
		break;

		case button::state::disabed:
			highlight(target, color, bounds, int2(3,3));
		break;
	}
}
