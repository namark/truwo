#ifndef IMPLEMENTAION_HPP
#define IMPLEMENTAION_HPP

#include <functional>
#include <vector>
#include "interface.hpp"
#include "simple.hpp"

//TODO: implementaion? come oooon... rename -_-

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

class ui_element : public i_interactive, public i_focusable
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
	std::vector<callback> on_release;

	void drop_focus() noexcept override;
	virtual void gain_focus() noexcept;
	bool focus() const noexcept override;
	bool focus_on(i_focusable*) noexcept override;
	bool focus(direction) noexcept override;

	protected:
	explicit ui_element(const i_bounds<int2>&);
	state current;

	private:
	// TODO: polymorphic bounds are clearly useless here, can just CRTP this
	const i_bounds<int2>& bounds_proxy;
	bool _focus;
};

template <typename Derived>
class focus_group : public i_focusable
{
	public:
	bool focus() const noexcept override;
	void drop_focus() noexcept override;
	bool focus_on(i_focusable*) noexcept override;
	bool focus(direction) override;

	private:
	Derived& drv() noexcept;
	const Derived& drv() const noexcept;
	bool focused_in_range() const noexcept;
};

class focus_vector : public focus_group<focus_vector>
{
	public:
	using container = std::vector<i_focusable*>;


	focus_vector(container elements);

	protected:
	auto& focus_range() noexcept;
	auto& focused() noexcept;
	const auto& focus_range() const noexcept;
	const auto& focused() const noexcept;
	friend class focus_group<focus_vector>;

	private:
	container elements;
	container::iterator focused_element;
};

#endif /* end of include guard */
