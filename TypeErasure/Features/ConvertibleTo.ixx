export module TypeErasure:Features.ConvertibleTo;

import :Erase;

import std;

namespace TypeErasure::ConvertibleDetail
{
	template <bool IsExplicit, typename... Types>
	struct ConvertibleTo
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (std::convertible_to<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto ConvertTo(void*, const std::type_info&) const -> void = 0;
		};

		template <typename M>
		class Model : public M
		{
			struct ConvertHelper
			{
				void* ptr;
				const Model& self;
				template <typename T> requires InParameterPack<T, Types...>
				auto Convert() const -> void
				{
					const auto& obj = self.GetObject();
					std::construct_at(static_cast<T*>(ptr), static_cast<T>(obj));
				}
			};

			public:
			using M::M;

			auto ConvertTo(void* out, const std::type_info& to) const -> void override
			{
				const auto result = ((to == typeid(Types) ? ConvertHelper{ out, *this }.template Convert<Types>(), true : false) || ... || false);
				if (!result)
				{
					throw std::bad_cast{ };
				}
			}
		};

		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto ConvertTo() const
			{
				auto vtable = dynamic_cast<VTable<VTableBase>*>(this->GetVTable());
				if (!vtable)
				{
					throw std::bad_any_cast{ };
				}

				alignas(T) std::array<std::byte, sizeof(T)> buffer{ };
				vtable->ConvertTo(static_cast<void*>(buffer.data()), typeid(T));

				auto* ptr = std::launder(reinterpret_cast<T*>(buffer.data()));

				struct Guard
				{
					T* ptr;
					~Guard() noexcept(std::is_nothrow_destructible_v<T>)
					{
						std::destroy_at(ptr);
					}
				} guard{ ptr };

				return std::move(*ptr);
			}

			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			explicit(IsExplicit) operator T() const
			{
				return ConvertTo<T>();
			}
		};
	};
}

export namespace TypeErasure
{
	template <typename... Types>
	using ExplicitlyConvertibleTo = ConvertibleDetail::ConvertibleTo<true, Types...>;

	template <typename... Types>
	using ImplicitlyConvertibleTo = ConvertibleDetail::ConvertibleTo<false, Types...>;
}
