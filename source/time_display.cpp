#include <cassert>
#include "time_display.h"
using namespace std::chrono;

time_display::time_display(int2 digit_size, int2 digit_spacing, graphical::color color) :
	ui_element(layout),
	digits{
		digit_bitmap(digit[0], color, rect{digit_size}),
		digit_bitmap(digit[0], color, rect{digit_size})
	},
	layout({&digits[0], &digits[1]}, digit_spacing)
{
	layout.update();
}

template <typename Type>
milliseconds time_display::set(milliseconds duration)
{
	auto trunc = duration_cast<Type>(duration);
	assert(trunc.count() < 100);
	digits[0].set_data(digit[trunc.count()/10]);
	digits[1].set_data(digit[trunc.count()%10]);
	return duration - trunc;
}
template milliseconds time_display::set<hours>(milliseconds duration);
template milliseconds time_display::set<minutes>(milliseconds duration);
template milliseconds time_display::set<seconds>(milliseconds duration);

time_display& time_display::operator+=(const int2& offset)
{
	layout += offset;
	return *this;
}

int2 time_display::lower() const
{
	return layout.lower();
}

int2 time_display::upper() const
{
	return layout.upper();
}

void time_display::draw(const graphical::surface& target)
{
	for(auto&& digit : digits)
		digit.draw(target);
}
