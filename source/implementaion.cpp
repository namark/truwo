#include "implementaion.h"

movable_bounds::movable_bounds(range2D bounds) : bounds(bounds) { }

int2 movable_bounds::lower() const
{
	return bounds.lower();
}

int2 movable_bounds::upper() const
{
	return bounds.upper();
}

movable_bounds& movable_bounds::operator+=(const int2& offset)
{
	bounds += offset;
	return *this;
}

ui_element::ui_element(const i_bounds<int2>& bounds) :
	current(state::idle),
	bounds_proxy(bounds),
	_focus(false)
{ }

bool contains(const i_bounds<int2>& bounds, int2 point)
{
	return bounds.lower() < point && point < bounds.upper();
};

void ui_element::update(const interactive::event& event) noexcept
{
	if(state::disabled == current)
		return;

	std::visit(support::overloaded{
		[this](const interactive::mouse_down& mouse)
		{
			if(contains(bounds_proxy, mouse.data.position))
			{
				current = state::pressed;
				for(auto&& callback : on_press)
					callback(*this);
			}
		},
		[this](const interactive::mouse_up& mouse)
		{
			if(state::pressed == current)
			{
				current = state::idle;
				if(contains(bounds_proxy, mouse.data.position))
				{
					current = state::hover;
					for(auto&& callback : on_click)
						callback(*this);
				}
			}
		},
		[this](const interactive::mouse_motion& mouse)
		{
			if(contains(bounds_proxy, mouse.data.position))
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

auto ui_element::current_state() const noexcept -> state
{
	return current;
}

void ui_element::disable(bool value) noexcept
{
	if(value && current != state::disabled)
		current = state::disabled;
	else
	if(!value && current == state::disabled)
		current = state::idle;
}

void ui_element::enable(bool value) noexcept
{
	disable(!value);
}

void ui_element::focus(bool value) noexcept
{
	_focus = value;
}

bool ui_element::focus() const noexcept
{
	return _focus;
}

void focus_group::focus()
{
	if(focused_element)
		focused_element->focus(true);
}

void focus_group::drop_focus()
{
	if(focused_element)
		focused_element->focus(false);
}

void focus_group::focus(size_t index)
{
	auto new_element = elements.at(index);
	if(focused_element)
		focused_element->focus(false);
	focused_element = new_element;
	focused_element->focus(true);
}

void focus_group::focus(i_focusable* element)
{
	expecting_focus = false;
	focus(find(elements.begin(), elements.end(), element) - elements.begin());
}

void focus_group::pre_update(const interactive::event& event)
{
	std::visit(support::overloaded{
		[this](const interactive::mouse_down&)
		{
			expecting_focus = true;
		},
		[](auto&&){}
	}, event);
}

void focus_group::post_update(const interactive::event& event)
{
	if(expecting_focus)
	{
		expecting_focus = false;
		drop_focus();
	}

	std::visit(support::overloaded{
		[this](const interactive::key_pressed& event)
		{
			using namespace interactive;
			if(event.data.keycode == interactive::keycode::tab)
			{
				int direction;
				if(pressed(scancode::rshift) || pressed(scancode::lshift))
				{
					direction = !focused_element
						? elements.size()-1
						: !focused_element->focus() ? 0 : -1;
				}
				else
				{
					direction = !focused_element || !focused_element->focus() ? 0 : 1;
				}

				focus(support::wrap(
					std::find(elements.begin(), elements.end(), focused_element) - elements.begin() +
					direction,
					elements.end() - elements.begin()
				));
			}
		},
		[](auto&&){}
	}, event);

}

