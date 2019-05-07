#ifndef BUTTON_H
#define BUTTON_H
#include "implementaion.h"
#include <functional>
#include <vector>

class button : public movable_bounds
{
	public:
	enum class state
	{
		idle,
		pressed,
		hover,
		disabed
	};

	button(range2D bounds);
	virtual ~button() = default;

	virtual void draw(const graphical::surface&) = 0;

	void update(interactive::event) noexcept;

	state current_state() const noexcept;

	void disable(bool = true) noexcept;
	void enable(bool = true) noexcept;

	using callback = std::function<void(button&)>;
	std::vector<callback> on_click;

	protected:
	state current;
};

#endif /* end of include guard */