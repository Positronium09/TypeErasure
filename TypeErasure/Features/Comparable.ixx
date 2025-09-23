export module TypeErasure:Features.Comparable;

import :Erase;

namespace Details
{
	template <typename T, typename U>
	concept LessThanCompareHelper = requires(const T & a, const U & b)
	{
		{ a < b } -> std::convertible_to<bool>;
	};
	template <typename T, typename U>
	concept LessThanOrEqualCompareHelper = requires(const T & a, const U & b)
	{
		{ a <= b } -> std::convertible_to<bool>;
	};
	template <typename T, typename U>
	concept GreaterThanCompareHelper = requires(const T & a, const U & b)
	{
		{ a > b } -> std::convertible_to<bool>;
	};
	template <typename T, typename U>
	concept GreaterThanOrEqualCompareHelper = requires(const T & a, const U & b)
	{
		{ a >= b } -> std::convertible_to<bool>;
	};
}

export namespace TypeErasure::Features
{
	struct EqualityComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = std::equality_comparable<T>;
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Equal(const VTable& other) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Equal(const VTable<VTableBase>& other) const -> bool override
			{
				return this->GetObject() == *other.GetObjectPtrCheckless<typename M::ObjectType>();
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator==(const Interface& other) const -> bool
			{
				if (!this->HasValue() || !other.HasValue())
				{
					return false;
				}
				if (this->Type() != other.Type())
				{
					return false;
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.Equal(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct EqualityComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (std::equality_comparable_with<T, Types> && ...);
		};

		template <typename V>
		struct VTable : virtual V
		{
			virtual auto CompareWith(const void*, const std::type_info&) const -> bool = 0;
		};

		template <typename M>
		struct Model : M
		{
			using M::M;
			auto CompareWith(const void* ptr, const std::type_info& type) const -> bool override
			{
				return ((type == typeid(Types) && this->GetObject() == *std::bit_cast<const Types*>(ptr)) || ...);
			}
		};

		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator==(const T& obj) const
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->CompareWith(&obj, typeid(T));
			}
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			friend auto operator==(const T& a, const Interface& b) -> bool
			{
				return b == a;
			}
		};
	};

	struct LessThanComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = requires(const T& a, const T& b)
			{
				{ a < b } -> std::convertible_to<bool>;
			};
		};

		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Less(const VTable& other) const -> bool = 0;
		};

		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Less(const VTable<VTableBase>& other) const -> bool override
			{
				return this->GetObject() < *other.GetObjectPtrCheckless<typename M::ObjectType>();
			}
		};

		template <typename I>
		struct Interface : virtual I
		{
			auto operator<(const Interface& other) const -> bool
			{
				if (!this->HasValue() || !other.HasValue())
				{
					throw std::bad_any_cast{};
				}
				if (this->Type() != other.Type())
				{
					throw std::bad_any_cast{};
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.Less(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct LessThanComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (Details::LessThanCompareHelper<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Less(const void*, const std::type_info&) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Less(const void* ptr, const std::type_info& type) const -> bool override
			{
				return ((type == typeid(Types) && this->GetObject() < *std::bit_cast<const Types*>(ptr)) || ...);
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator<(const T& obj) const
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->Less(&obj, typeid(T));
			}
		};
	};

	struct LessThanOrEqualComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = requires(const T& a, const T& b)
			{
				{ a <= b } -> std::convertible_to<bool>;
			};
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto LessEqual(const VTable& other) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto LessEqual(const VTable<VTableBase>& other) const -> bool override
			{
				return this->GetObject() <= *other.GetObjectPtrCheckless<typename M::ObjectType>();
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator<=(const Interface& other) const -> bool
			{
				if (!this->HasValue() || !other.HasValue())
				{
					throw std::bad_any_cast{};
				}
				if (this->Type() != other.Type())
				{
					throw std::bad_any_cast{};
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.LessEqual(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct LessThanOrEqualComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (Details::LessThanOrEqualCompareHelper<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto LessEqual(const void*, const std::type_info&) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto LessEqual(const void* ptr, const std::type_info& type) const -> bool override
			{
				return ((type == typeid(Types) && this->GetObject() <= *std::bit_cast<const Types*>(ptr)) || ...);
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator<=(const T& obj) const
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->LessEqual(&obj, typeid(T));
			}
		};
	};

	struct GreaterThanComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = requires(const T& a, const T& b)
			{
				{ a > b } -> std::convertible_to<bool>;
			};
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Greater(const VTable& other) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Greater(const VTable<VTableBase>& other) const -> bool override
			{
				return this->GetObject() > *other.GetObjectPtrCheckless<typename M::ObjectType>();
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator>(const Interface& other) const -> bool
			{
				if (!this->HasValue() || !other.HasValue())
				{
					throw std::bad_any_cast{};
				}
				if (this->Type() != other.Type())
				{
					throw std::bad_any_cast{};
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.Greater(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct GreaterThanComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (Details::GreaterThanCompareHelper<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Greater(const void*, const std::type_info&) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Greater(const void* ptr, const std::type_info& type) const -> bool override
			{
				return ((type == typeid(Types) && this->GetObject() > *std::bit_cast<const Types*>(ptr)) || ...);
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator>(const T& obj) const
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->Greater(&obj, typeid(T));
			}
		};
	};

	struct GreaterThanOrEqualComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = requires(const T& a, const T& b)
			{
				{ a >= b } -> std::convertible_to<bool>;
			};
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto GreaterEqual(const VTable& other) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto GreaterEqual(const VTable<VTableBase>& other) const -> bool override
			{
				return this->GetObject() >= *other.GetObjectPtrCheckless<typename M::ObjectType>();
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator>=(const Interface& other) const -> bool
			{
				if (!this->HasValue() || !other.HasValue())
				{
					return false;
				}
				if (this->Type() != other.Type())
				{
					throw std::bad_any_cast{};
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.GreaterEqual(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct GreaterThanOrEqualComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (Details::GreaterThanOrEqualCompareHelper<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto GreaterEqual(const void*, const std::type_info&) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto GreaterEqual(const void* ptr, const std::type_info& type) const -> bool override
			{
				return ((type == typeid(Types) && this->GetObject() >= *std::bit_cast<const Types*>(ptr)) || ...);
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator>=(const T& obj) const
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->GreaterEqual(&obj, typeid(T));
			}
		};
	};

	struct ThreeWayComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = std::three_way_comparable<T>;
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto ThreeWay(const VTable& other) const -> std::partial_ordering = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto ThreeWay(const VTable<VTableBase>& other) const -> std::partial_ordering override
			{
				return std::partial_order(this->GetObject(), *other.GetObjectPtrCheckless<typename M::ObjectType>());
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator<=>(const Interface& other) const noexcept -> std::partial_ordering
			{
				if (!this->HasValue() || !other.HasValue())
				{
					return std::partial_ordering::unordered;
				}
				if (this->Type() != other.Type())
				{
					return std::partial_ordering::unordered;
				}
				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>&>(*other.GetVTable());
				return vTable.ThreeWay(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct ThreeWayComparableWith
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = (std::three_way_comparable_with<T, Types> && ...);
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto ThreeWay(const void*, const std::type_info&) const -> std::partial_ordering = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto ThreeWay(const void* ptr, const std::type_info& type) const -> std::partial_ordering override
			{
				std::partial_ordering result = std::partial_ordering::unordered;
				((type == typeid(Types) ? result = std::partial_order(this->GetObject(), *std::bit_cast<const Types*>(ptr)) : void()), ...);
				return result;
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			template <typename T> requires InParameterPack<T, Types...> && !IsSpecialization<T, Any>
			auto operator<=>(const T& obj) const noexcept -> std::partial_ordering
			{
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->ThreeWay(&obj, typeid(T));
			}
		};
	};
}
