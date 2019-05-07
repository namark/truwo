#ifndef BITMAP_HPP
#define BITMAP_HPP

#include "implementaion.h"

template <int width, int height>
class bitmap_data
{
	public:
	static constexpr int2 size = int2{width, height}; // this should be a template parameter
	using bitset = std::bitset<size.x() * size.y()>;
	bitset bits;
};

template <int width, int height>
class bitmap : public movable_bounds
{
	public:
	using bit_data = bitmap_data<width, height>;
	bitmap(const bit_data& data, graphical::color color, range2D bounds) :
		movable_bounds(bounds), data(data), color(color)
	{ }

	void draw(const graphical::surface& surface)
	{
		const auto size = bit_data::size;
		const auto cell_size = (bounds.upper() - bounds.lower())/size;
		auto cell = int2::zero();
		auto dimension = cell.begin();
		while(dimension != cell.end())
		{
			if(data.bits[data.bits.size()-1 - (cell.y() * size.x() + cell.x())])
				fill(surface, color, rect{cell_size, bounds.lower() + cell*cell_size});
			dimension = support::advance_vector(cell, int2::zero(), size);
		}
	}

	private:
	const bit_data& data;
	graphical::color color;
};

template <int width, int height>
bitmap(const bitmap_data<width, height>&, range2D, graphical::color)
	-> bitmap<width, height>;

#endif /* end of include guard */
