#ifndef UI_FACTORY_HPP
#define UI_FACTORY_HPP

#include <vector>
#include <memory>

#include "simple/support/tuple_utils.hpp"

//TODO; nothing UI here anymore... also not really a factory and not really a shop... rename all the things!

template <typename... Interfaces>
class ui_shop;

template <typename Type, typename... Interfaces>
class ui_factory
{
	public:

	using type = Type;

	template <typename Element, typename ...Args>
	Element& make(Args&&... args)
	{
		using namespace simple::support;
		auto& elements = tuple_car(interfaces);
		auto element = std::make_unique<Element>(std::forward<Args>(args)...);
		Element& raw = *element.get();

		add_interface(raw, tuple_tie_cdr(interfaces));

		elements.push_back(std::move(element)); // assuming raw pointer will not change... that's safe right?
		return raw;
	}

	template <typename Interface>
	const auto& get() const noexcept
	{
		using ptr_t = std::conditional_t<std::is_same_v<Interface, type>,
			  std::unique_ptr<type>,
			  Interface*>;
		return std::get<std::vector<ptr_t>>(interfaces);
	}

	private:

	template <typename... T>
	using container = std::tuple< std::vector<T> ... >;
	template <typename... T>
	using container_ref = std::tuple< std::vector<T>& ... >;

	container<
		std::unique_ptr<type>,
		Interfaces* ...
	> interfaces;


	template <typename El>
	void add_interface(El&, std::tuple<>){}

	template <typename El, typename In, typename... Rest>
	void add_interface(El& element, container_ref<In*, Rest...> interfaces)
	{
		using namespace simple::support;
		if constexpr (std::is_base_of_v<In, El>)
			tuple_car(interfaces).push_back(&element);
		add_interface(element,tuple_tie_cdr(interfaces));
	}

	friend class ui_shop<Type, Interfaces...>;
};

template <typename... Interfaces>
class ui_shop
{
	public:

	using receipt_id_t = unsigned;

	ui_shop(ui_factory<Interfaces...> supplier) :
		supplier(std::move(supplier))
	{
	}

	template <typename Goods>
	struct order
	{
		public:
		Goods goods;
		const receipt_id_t receipt_id;

		private:
		order(Goods&& goods, const receipt_id_t& receipt_id) :
			goods(std::forward<Goods>(goods)),
			receipt_id(receipt_id)
		{}
		friend class ui_shop;
	};

	template <typename Function>
	order<std::invoke_result_t<
		Function, ui_factory<Interfaces...>&>>
	make_order(Function&& function)
	{
		using simple::support::transform;
		auto size = transform([](const auto& x) {return x.size(); },
			supplier.interfaces);

		auto&& goods = std::invoke(
			std::forward<Function>(function),
			supplier);

		receipt_ids.push_back(get_id());

		transform([](auto& receipt_size, auto size, const auto& interface)
		{
			receipt_size.push_back(interface.size() - size);
		}, receipt_sizes, size, supplier.interfaces);

		return
		{
			std::forward<decltype(goods)>(goods),
			receipt_ids.back()
		};
	};

	template <typename Element, typename ...Args>
	Element& make(Args&&... args)
	{
		return make_order([&](auto& factory) -> auto&
		{
			return
				factory.template make<Element>
					(std::forward<Args>(args)...);
		}).goods;
	}

	bool recycle(receipt_id_t id)
	{
		auto found = lower_bound
		(
			begin(receipt_ids),
			end(receipt_ids),
			id
		);

		if(found != end(receipt_ids))
		{
			using simple::support::transform;
			transform([&](auto& interfaces, auto& sizes)
			{
				auto size_begin = begin(receipt_sizes);
				auto found_size = size_begin +
					(found - begin(receipt_ids));

				auto goods_begin = begin(interfaces) +
					accumulate(size_begin, found_size, 0);
				interfaces.erase
				(
					goods_begin,
					goods_begin + *found_size
				);


				sizes.erase(found_size);
			}, supplier.interfaces, receipt_sizes);

			receipt_ids.erase(found);
			return true;
		}

		return false;
	}

	template <typename Interface>
	decltype(auto) get() const noexcept
	{ return supplier.template get<Interface>(); }

	private:

	using receipt_size_t = unsigned short;

	receipt_id_t get_id()
	{
		if(empty(receipt_ids))
			return 0;
		return receipt_ids.back() + 1; //TODO: what if we run out?
		// but it'll take like two hundred bajillion years D':
		// yes, what will this program do after two hundred bajillion years of runtime?
		// reuse ids? do I smell std::rotate? =)
	}

	ui_factory<Interfaces...> supplier;
	std::vector<receipt_id_t> receipt_ids;
	std::array<
		std::vector<receipt_size_t>,
		sizeof...(Interfaces) + 1
	> receipt_sizes;
};


#endif /* end of include guard */
