#ifndef IMPLEMENTATION_HPP
#define IMPLEMENTATION_HPP
#include "implementation.h"

template <typename Derived>
Derived& focus_group<Derived>::drv() noexcept
{
	return *static_cast<Derived*>(this);
}

template <typename Derived>
const Derived& focus_group<Derived>::drv() const noexcept
{
	return *static_cast<const Derived*>(this);
}

template <typename Derived>
bool focus_group<Derived>::focused_in_range() const noexcept
{
	using std::end;
	return drv().focused() != end(drv().focus_range());
}

template <typename Derived>
bool focus_group<Derived>::focus() const noexcept
{
	using std::end;
	return focused_in_range() &&
		(*drv().focused())->focus();
}

template <typename Derived>
void focus_group<Derived>::drop_focus() noexcept
{
	using std::end;
	if( focused_in_range() )
		(*drv().focused())->drop_focus();
}

template <typename Derived>
bool focus_group<Derived>::focus_on(i_focusable* target) noexcept
{
	using std::begin;
	using std::end;

	if(focused_in_range())
		(*drv().focused())->drop_focus();

	for
	(
		auto i = begin( drv().focus_range() );
		i != end( drv().focus_range() );
		++i
	)
	{
		if((*i)->focus_on(target))
		{
			drv().focused() = i;
			return true;
		}
	}
	return false;
}

template <typename Derived>
bool focus_group<Derived>::focus(direction dir)
{
	using std::begin;
	using std::end;

	auto& focused = drv().focused();
	auto& all = drv().focus_range();

	if(!focus())
	{
		focused = dir >= 0
			? begin(all)
			: end(all) - 1
		;
		(*focused)->focus(dir);
		return true;
	}

	if((*focused)->focus(dir))
		return true;

	auto next = focused + dir;

	if( begin(all) <= next && next < end(all) )
	{
		(*focused)->drop_focus();
		focused = next;
		(*focused)->focus(dir);
		return true;
	}

	return false;
}
#endif /* end of include guard */
