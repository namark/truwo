#ifndef TIME_DISPLAY_H
#define TIME_DISPLAY_H

#include <cassert>
#include <utility>
#include "implementation.h"
#include "layout.h"
#include "digits.h"

template <size_t digit_count = 2>
class digit_display :
	public i_movable_bounds<int2>,
	public i_graphic,
	public ui_element,
	public i_ui_object
{
	public:

	digit_display(int2 digit_size, int2 digit_spacing, graphical::color color);
	virtual ~digit_display() = default;

	void set(int value) noexcept;

	digit_display& operator+=(const int2&) override;
	int2 lower() const override;
	int2 upper() const override;

	void draw(const graphical::surface&) override;
	void update(const interactive::event& event) noexcept override;

	void drop_focus() noexcept override;

	using callback = std::function<void(digit_display&, int old_value, int new_value)>;
	std::vector<callback> on_input;

	private:
	template <size_t... I>
	digit_display(int2 digit_size, int2 digit_spacing, graphical::color color, std::index_sequence<I...>);
	graphical::color color;
	std::array<digit_bitmap, digit_count> digits;
	bounds_layout layout;
	int set_value;
	int input_value;
	bool input;
};

#endif /* end of include guard */
