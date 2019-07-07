#include "plain_button.h"
#include "utils.hpp"

plain_button::plain_button(graphical::color color, range2D bounds) :
	movable_bounds(bounds),
	ui_element(static_cast<i_bounds<int2>&>(*this)),
	color(color)
{ }

void plain_button::draw(const graphical::surface& target)
{
	switch(current_state())
	{
		case ui_element::state::hover:
			lowlight(target, color, bounds, int2(2,2));
		[[fallthrough]]; // so ugly, my poor switch :(
		case ui_element::state::idle:
			outline(target, color, bounds);
		break;

		case ui_element::state::pressed:
			graphical::fill(target, color, bounds);
		break;

		case ui_element::state::disabled:
			lowlight(target, color, bounds, int2(3,3));
		break;
	}

	if(focus())
	{
		const auto padding = support::min_element(upper() - lower())/10;
		if(current_state() == ui_element::state::disabled)
			outline(target, color, {lower() + padding, upper() - padding});
		else
			outline_lowlight(target, color, {lower() + padding, upper() - padding});
	}
}

void plain_button::update(const interactive::event& event) noexcept
{
	ui_element::update(event);

	if(current_state() == ui_element::state::disabled)
		return;

	if(focus())
	{
		using namespace interactive;
		std::visit(support::overloaded
		{
			[this](const key_pressed& event)
			{
				if(event.data.scancode == scancode::space && event.data.repeat == 0)
				{
					current = state::pressed;
					for(auto&& callback : on_press)
						callback(*this);
				}
			},
			[this](const key_released& event)
			{
				if(event.data.scancode == scancode::space)
				{
					current = state::idle;
					for(auto&& callback : on_release)
						callback(*this);
					for(auto&& callback : on_click)
						callback(*this);
				}
			},
			[](auto&&){}
		}, event);
	}
}
