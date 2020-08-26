#ifndef TIME_DISPLAY_HPP
#define TIME_DISPLAY_HPP

#include <cassert>
#include "digit_display.h"
#include "utils.hpp"

// what is integer exp that you don't have it, eh? c++, eh? -_-
// TODO: add integer exp to simple::support i guess
constexpr int int_pow(int i, int p)
{
	int r = 1;
	while(p-->0)
		r *= i;
	return r;
}

template <size_t digit_count>
digit_display<digit_count>::
	digit_display(int2 digit_size, int2 digit_spacing, graphical::color color) :
		digit_display(
			digit_size, digit_spacing, color, std::make_index_sequence<digit_count>())
{}

template <size_t digit_count>
template <size_t... I>
digit_display<digit_count>::digit_display(int2 digit_size, int2 digit_spacing, graphical::color color, std::index_sequence<I...>) :
	ui_element(layout),
	color(color),
	digits{ (void(I),
		digit_bitmap(digit[0], color, rect{digit_size})
	)...},
	layout({&digits[I]...}, digit_spacing),
	set_value(0),
	input_value(0),
	input(false)
{
	layout.update();
	digits[0].align = -int2::i();
	digits[digit_count - 1].align = int2::i();
}

template <size_t digit_count>
void digit_display<digit_count>::set(int value) noexcept
{
	assert(0 <= value && value < int_pow(10, digit_count));
	set_value = value;
	for(size_t i = digit_count; i --> 0;)
	{
		digits[i].set_data(digit[value%10]);
		value /= 10;
	}
}

template <size_t digit_count>
digit_display<digit_count>& digit_display<digit_count>::operator+=(const int2& offset)
{
	layout += offset;
	return *this;
}

template <size_t digit_count>
int2 digit_display<digit_count>::lower() const
{
	return layout.lower();
}

template <size_t digit_count>
int2 digit_display<digit_count>::upper() const
{
	return layout.upper();
}

template <size_t digit_count>
void digit_display<digit_count>::draw(const graphical::surface& target)
{
	if(ui_element::focus())
	{
		lowlight(target, color, {lower(), upper()}, int2{3,3});
		graphical::fill(target, color, {upper() - (upper()-lower()) / int2{1, 40}, upper()});
	}

	for(auto&& digit : digits)
		digit.draw(target);
}

template <size_t digit_count>
void digit_display<digit_count>::update(const interactive::event& event) noexcept
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

						if(input_value < int_pow(10, digit_count - 1))
						{
							input_value = input_value * 10 + digit;
							input_value %= int_pow(10, digit_count);
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

template <size_t digit_count>
void digit_display<digit_count>::drop_focus() noexcept
{
	ui_element::drop_focus();
	input = false;
	input_value = 0;
}

#endif /* end of include guard */
