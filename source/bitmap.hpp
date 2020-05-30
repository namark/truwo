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
		movable_bounds(bounds), dim(), align(), data(&data), color(color)
	{
		calculate_padding();
	}

	void draw(const graphical::surface& surface)
	{
		const auto size = bit_data::size;
		const auto cell_size = (bounds.upper() - bounds.lower())/size;

		// alignment logic
		assert(to_conjunction( (align == -int2::one()) | (align == int2::zero()) | (align == int2::one()) ));
		const int2 lower_mask (align == -int2::one());
		const auto upper_mask = 1 - lower_mask;
		const auto align_offset = cell_size * align * (lower_mask * padding.lower() + upper_mask * padding.upper());

		auto cell = int2::zero();
		auto dimension = cell.begin();
		while(dimension != cell.end())
		{
			if(get_bit(cell))
			{
				rect cell_rect {cell_size, bounds.lower() + cell*cell_size};
				cell_rect.position += align_offset;

				if(not(dim <= int2::zero()))
				{
					lowlight(surface, color, cell_rect, dim);
				}
				else
				{
					fill(surface, color, cell_rect);
				}
			}
			dimension = support::advance_vector(cell, int2::zero(), size);
		}
	}

	void set_data(const bit_data& new_data)
	{
		if(data != &new_data)
		{
			data = &new_data;
			calculate_padding();
		}
	}

	int2 dim;
	int2 align;

	private:
	const bit_data* data;
	graphical::color color;
	range2D padding;

	void calculate_padding()
	{
		// inefficient, but shouldn't happen very often,
		// don't generalize this as is though
		auto cell = int2::zero();
		auto dimension = cell.begin();
		auto cropped = invalid_range;
		while(dimension != cell.end())
		{
			if(get_bit(cell))
			{
				cropped.upper().max(cell);
				cropped.lower().min(cell);
			}
			dimension = support::advance_vector(cell, int2::zero(), bit_data::size);
		}
		padding.lower() = cropped.lower();
		padding.upper() = bit_data::size - 1 - cropped.upper(); // ugly -1 cause bits have width
	}

	bool get_bit(int2 cell)
	{
		return data->bits[data->bits.size()-1 - (cell.y() * bit_data::size.x() + cell.x())];
	}

};

template <int width, int height>
bitmap(const bitmap_data<width, height>&, range2D, graphical::color)
	-> bitmap<width, height>;

#endif /* end of include guard */
