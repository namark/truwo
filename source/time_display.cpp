#include <cassert>
#include "time_display.h"
#include "utils.hpp"

using namespace std::chrono;

time_display::time_display(int2 digit_size, int2 digit_spacing, graphical::color color) :
	ui_element(layout),
	color(color),
	digits{
		digit_bitmap(digit[0], color, rect{digit_size}),
		digit_bitmap(digit[0], color, rect{digit_size})
	},
	layout({&digits[0], &digits[1]}, digit_spacing),
	set_value(0),
	input_value(0),
	input(false)
{
	layout.update();
}

void time_display::set(int value) noexcept
{
	assert(0 <= value && value < 100);
	digits[0].set_data(digit[value/10]);
	digits[1].set_data(digit[value%10]);
	set_value = value;
}

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
	if(ui_element::focus())
	{
		lowlight(target, color, {lower(), upper()}, int2{3,3});
		graphical::fill(target, color, {upper() - (upper()-lower()) / int2{1, 40}, upper()});
	}

	for(auto&& digit : digits)
		digit.draw(target);
}

void time_display::update(const interactive::event& event) noexcept
{
	ui_element::update(event);

	if(current_state() == ui_element::state::disabled)
		return;

	if(ui_element::focus())
	{
		using namespace interactive;
		std::visit(support::overloaded
		{
			[this](const key_pressed& key_event)
			{
				int digit = -1;

				switch(key_event.data.keycode)
				{
					case keycode::_9:
					case keycode::kp_9:
					++digit;
					[[fallthrough]]; // T_T

					case keycode::_8:
					case keycode::kp_8:
					++digit;
					[[fallthrough]]; // T-T

					case keycode::_7:
					case keycode::kp_7:
					++digit;
					[[fallthrough]]; // T-T

					case keycode::_6:
					case keycode::kp_6:
					++digit;
					[[fallthrough]]; // ToT

					case keycode::_5:
					case keycode::kp_5:
					++digit;
					[[fallthrough]]; // TOT

					case keycode::_4:
					case keycode::kp_4:
					++digit;
					[[fallthrough]]; // ToT

					case keycode::_3:
					case keycode::kp_3:
					++digit;
					[[fallthrough]]; // T-T

					case keycode::_2:
					case keycode::kp_2:
					++digit;
					[[fallthrough]]; // T-T

					case keycode::_1:
					case keycode::kp_1:
					++digit;
					[[fallthrough]]; // T_T

					case keycode::_0:
					case keycode::kp_0:
					++digit;
					{
						if(!input)
						{
							input = true;
							input_value = 0;
						}

						if(input_value < 10)
						{
							input_value = input_value * 10 + digit;
							size_t exp = 1;
							for(auto&& this_[[maybe_unused]] : digits) exp *= 10;
							input_value %= exp;
							// what is integer exp that you don't have it, eh? c++, eh? -_-
							// TODO: add integer exp to simple::support i guess
						}
					}
					break;

					case keycode::backspace:
						input_value /= 10;
					break;


					default: return;
				}

				if(input_value != set_value)
				{
					for(auto&& callback : on_input)
						callback(*this,  set_value, input_value);
					set_value = input_value;
				}

			},
			[](auto&&){}
		}, event);

	}
}

void time_display::focus(bool value) noexcept
{
	ui_element::focus(value);

	if(!ui_element::focus())
	{
		input = false;
		input_value = 0;
	}
}
