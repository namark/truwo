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
			highlight(target, color, bounds, int2(2,2));
		[[fallthrough]]; // so ugly, my poor switch :(
		case ui_element::state::idle:
			outline(target, color, bounds);
		break;

		case ui_element::state::pressed:
			graphical::fill(target, color, bounds);
		break;

		case ui_element::state::disabled:
			highlight(target, color, bounds, int2(3,3));
		break;
	}
}
