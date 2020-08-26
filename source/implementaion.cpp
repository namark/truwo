#include "implementaion.h"
#include "implementation.hpp"

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
				for(auto&& callback : on_release)
					callback(*this);
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

void ui_element::drop_focus() noexcept
{
	_focus = false;
}

void ui_element::gain_focus() noexcept
{
	_focus = true;
}

bool ui_element::focus() const noexcept
{
	return _focus;
}

bool ui_element::focus_on(i_focusable* target) noexcept
{
	if(target == this)
	{
		gain_focus();
		return true;
	}
	return false;
}

bool ui_element::focus(direction) noexcept
{
	if(!focus())
	{
		gain_focus();
		return true;
	}
	return false;
}

focus_vector::focus_vector(container elements) :
	elements(std::move(elements)),
	focused_element(this->elements.end())
{}

auto& focus_vector::focus_range() noexcept
{
	return elements;
}

auto& focus_vector::focused() noexcept
{
	return focused_element;
}

const auto& focus_vector::focus_range() const noexcept
{
	return elements;
}

const auto& focus_vector::focused() const noexcept
{
	return focused_element;
}
