#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "simple.hpp"

template <typename Offset>
class i_movable
{
	protected:
	~i_movable() = default;
	public:
	virtual i_movable& operator+=(const Offset&) = 0;
};

template <typename Bound>
class i_bounds
{
	protected:
	~i_bounds() = default;
	public:
	virtual Bound lower() const = 0;
	virtual Bound upper() const = 0;
};

template <typename T>
class i_movable_bounds : public i_bounds<T>, public i_movable<T>
{
	protected:
	~i_movable_bounds() = default;
};

class i_graphic
{
	protected:
	~i_graphic() = default;
	public:
	virtual void draw(const graphical::surface&) = 0;
};

class i_interactive
{
	protected:
	~i_interactive() = default;
	public:
	virtual void update(const interactive::event&) = 0;
};

class i_focusable
{
	protected:
	~i_focusable() = default;
	public:

	enum direction
	{
		prev = -1,
		self = 0,
		next = 1
	};

	virtual bool focus() const = 0;
	virtual void drop_focus() = 0;
	virtual bool focus(direction) = 0;
	virtual bool focus_on(i_focusable*) = 0;
};

class i_ui_object
{
	public:
	virtual ~i_ui_object() = default;
};

#endif /* end of include guard */
