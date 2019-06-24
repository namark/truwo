#ifndef TIME_DISPLAY_H
#define TIME_DISPLAY_H

#include <cassert>
#include "implementaion.h"
#include "layout.h"
#include "digits.h"

class time_display :
	public i_movable_bounds<int2>,
	public i_graphic,
	public ui_element,
	public i_ui_object
{
	public:
	using duration = std::chrono::steady_clock::duration;

	time_display(int2 digit_size, int2 digit_spacing, graphical::color color);
	virtual ~time_display() = default;

	template<typename Type>
	duration set(duration);

	time_display& operator+=(const int2&) override;
	int2 lower() const override;
	int2 upper() const override;

	void draw(const graphical::surface&) override;
	void update(const interactive::event& event) noexcept override;

	void focus(bool) noexcept override;

	using callback = std::function<void(time_display&, int old_value, int new_value)>;
	std::vector<callback> on_input;

	private:
	graphical::color color;
	std::array<digit_bitmap, 2> digits;
	bounds_layout layout;
	int set_value;
	int input_value;
	bool input;
};

#endif /* end of include guard */
