#ifndef TIME_DISPLAY_H
#define TIME_DISPLAY_H

#include <cassert>
#include "implementaion.h"
#include "layout.h"
#include "digits.h"

class time_display :
	public i_movable_bounds<int2>,
	public i_graphic,
	public ui_element,
	public i_ui_object
{
	public:
	enum class state
	{
		normal,
		input,
		hover,
		disabled
	};

	time_display(int2 digit_size, int2 digit_spacing, graphical::color color);
	virtual ~time_display() = default;

	template<typename Type>
	std::chrono::milliseconds set(std::chrono::milliseconds);

	time_display& operator+=(const int2&) override;
	int2 lower() const override;
	int2 upper() const override;

	void draw(const graphical::surface&) override;

	private:
	std::array<digit_bitmap, 2> digits;
	bounds_layout layout;
};

#endif /* end of include guard */
