#ifndef BITMAP_HPP
#define BITMAP_HPP

#include "implementaion.h"
#include "utils.hpp"

template <int width, int height>
class bitmap_data
{
	public:
	static constexpr int2 size = int2{width, height}; // this should be a template parameter
	using bitset = std::bitset<size.x() * size.y()>;
	bitset bits;
};

template <int width, int height>
class bitmap : public movable_bounds, public i_graphic, public i_ui_object
{
	public:
	using bit_data = bitmap_data<width, height>;
	bitmap(const bit_data& data, graphical::color color, range2D bounds) :
		movable_bounds(bounds), dim(int2::zero()), data(&data), color(color)
	{ }

	void draw(const graphical::surface& surface)
	{
		const auto size = bit_data::size;
		const auto cell_size = (bounds.upper() - bounds.lower())/size;
		auto cell = int2::zero();
		auto dimension = cell.begin();
		while(dimension != cell.end())
		{
			if(data->bits[data->bits.size()-1 - (cell.y() * size.x() + cell.x())])
			{
				// arguably any_of(dim, greather_that_zero) is better, but i like this
				if(not(dim <= int2::zero()))
				{
					lowlight(surface, color, rect{cell_size, bounds.lower() + cell*cell_size}, dim);
				}
				else
				{
					fill(surface, color, rect{cell_size, bounds.lower() + cell*cell_size});
				}
			}
			dimension = support::advance_vector(cell, int2::zero(), size);
		}
	}

	void set_data(const bit_data& new_data) { data = &new_data; }

	int2 dim;

	private:
	const bit_data* data;
	graphical::color color;
};

template <int width, int height>
bitmap(const bitmap_data<width, height>&, range2D, graphical::color)
	-> bitmap<width, height>;

#endif /* end of include guard */
