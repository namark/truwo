#include "button.h"

button::button(range2D bounds) :
	bounds(bounds),
	current(state::idle)
{ }

void button::update(interactive::event event) noexcept
{
	if(state::disabed == current)
		return;

	std::visit(support::overloaded{
		[this](interactive::mouse_down mouse)
		{
			if(bounds.contains(mouse.data.position))
			{
				current = state::pressed;
			}
		},
		[this](interactive::mouse_up mouse)
		{
			if(state::pressed == current)
			{
				current = state::idle;
				if(bounds.contains(mouse.data.position))
				{
					current = state::hover;
					for(auto&& callback : on_click)
						callback(*this);
				}
			}
		},
		[this](interactive::mouse_motion mouse)
		{
			if(bounds.contains(mouse.data.position))
			{
				if(state::idle == current )
					current = state::hover;
			}
			else
			{
				if(state::hover == current )
					current = state::idle;
			}
		},
		[](auto) { }
	}, event);
}

auto button::current_state() const noexcept -> state
{
	return current;
}

void button::disable(bool value) noexcept
{
	current = value ? state::disabed : state::idle;
}

void button::enable(bool value) noexcept
{
	disable(!value);
}

