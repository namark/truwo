#ifndef IMPLEMENTAION_HPP
#define IMPLEMENTAION_HPP

#include <functional>
#include <vector>
#include "interface.hpp"
#include "simple.hpp"

class movable_bounds : public i_movable_bounds<int2>
{
	public:
	movable_bounds(range2D bounds);

	int2 lower() const override;
	int2 upper() const override;
	movable_bounds& operator+=(const int2& offset) override;

	virtual ~movable_bounds() = default;

	protected:
	movable_bounds(const movable_bounds&) = default;
	movable_bounds& operator=(const movable_bounds&) = default;
	range2D bounds;

};

class ui_element : public i_interactive_focusable
{
	public:
	enum class state
	{
		idle,
		pressed,
		hover,
		disabled
	};

	virtual ~ui_element() = default;

	void update(const interactive::event&) noexcept override;

	state current_state() const noexcept;

	void disable(bool = true) noexcept;
	void enable(bool = true) noexcept;

	using callback = std::function<void(ui_element&)>;
	std::vector<callback> on_click;
	std::vector<callback> on_press;

	void focus(bool) noexcept override;
	bool focus() const noexcept override;

	protected:
	explicit ui_element(const i_bounds<int2>&);

	private:
	const i_bounds<int2>& bounds_proxy;
	state current;
	bool _focus;
};

class focus_group
{
	public:
	using container = std::vector<i_focusable*>;

	void focus();
	void drop_focus();
	void focus(size_t index);
	void focus(i_focusable*);

	void pre_update(const interactive::event&);
	void post_update(const interactive::event&);

	container elements;
	private:
	i_focusable* focused_element = nullptr;
	bool expecting_focus = false;
};

#endif /* end of include guard */
