#include "layout.h"

int2 select_corner(range2D rng, int2 direction) noexcept
{
	// center + direction * half_size
	return
	(
		(rng.lower() + rng.upper()) // center * 2
		+ direction * (rng.upper() - rng.lower()) // direction * size
	) / 2; // all halved
}

bounds_layout& bounds_layout::operator+=(const int2& offset)
{
	movable_bounds::operator+=(offset);
	for(auto&& element : elements)
		*element += offset;
	return *this;
}

template <typename Itr>
class double_deref_itr
{
	public:
	Itr itr;
	double_deref_itr& operator++() noexcept
	{
		++itr;
		return *this;
	}

	double_deref_itr operator++(int) noexcept
	{
		double_deref_itr tmp{ itr++ };
		return tmp;
	}

	double_deref_itr operator+(int i) noexcept { return {itr + i}; }
	double_deref_itr operator-(int i) noexcept { return {itr - i}; }

	auto&& operator*() { return *(*itr); }
	bool operator==(double_deref_itr other) { return itr == other.itr; }
	bool operator!=(double_deref_itr other) { return !(*this == other); }
};
template <typename T> double_deref_itr(T) -> double_deref_itr<T>;

range2D bounds_layout::update()
{
	layout_bounds(double_deref_itr{elements.begin()}, double_deref_itr{elements.end()}, spacing);

	// invalid range, returned for empty layout
	bounds = invalid_range;
	for(auto&& element : elements)
	{
		bounds.lower() = min(bounds.lower(), element->lower());
		bounds.upper() = max(bounds.upper(), element->upper());
	}
	return bounds;
}
