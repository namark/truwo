#include "button.h"

class plain_button : public button
{
	public:
	explicit plain_button(const plain_button&) = default;
	plain_button(graphical::color color, range2D bounds);
	virtual void draw(const graphical::surface&) override;

	private:
	graphical::color color;
};
