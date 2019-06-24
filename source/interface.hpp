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
	virtual void focus(bool) = 0;
	virtual bool focus() const = 0;
};

class i_interactive_focusable : public i_interactive, public i_focusable
{
	protected:
	~i_interactive_focusable() = default;
};

class i_ui_object
{
	public:
	virtual ~i_ui_object() = default;
};

#endif /* end of include guard */
