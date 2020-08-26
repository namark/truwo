#include "implementation.h"

class plain_button :
	public movable_bounds,
	public ui_element,
	public i_graphic,
	public i_ui_object
{
	public:
	explicit plain_button(const plain_button&) = default;
	plain_button(graphical::color color, range2D bounds);
	virtual void draw(const graphical::surface&) override;
	virtual void update(const interactive::event&) noexcept override;

	private:
	graphical::color color;
};
