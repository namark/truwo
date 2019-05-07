#ifndef INTERFACE_HPP
#define INTERFACE_HPP

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

#endif /* end of include guard */
