export module TypeErasure:Features.Comparable;

import :Erase;

export namespace TypeErasure::Features
{
	struct EqualityComparable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = requires(const T& a, const T& b)
			{
				{ a == b } -> std::convertible_to<bool>;
				{ a != b } -> std::convertible_to<bool>;
			};
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Equal(const VTable* other) const -> bool = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Equal(const VTable<VTableBase>* other) const -> bool override
			{
				const auto* otherModel = dynamic_cast<const Model*>(other);
				return this->GetObject() == otherModel->GetObject();
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
				const auto& otherVTable = dynamic_cast<const VTable<VTableBase>*>(other.GetVTable());
				return vTable.Equal(otherVTable);
			}
		};
	};

	template <typename... Types>
	struct EqualityComparableWith
	{
		template <typename T>
		class Validator
		{
			template <typename T1, typename T2>
			struct CanCompare
			{
				static constexpr auto value = requires(const T1& a, const T2& b)
				{
					{ a == b } -> std::convertible_to<bool>;
					{ a != b } -> std::convertible_to<bool>;
				};
			};

			public:
			static constexpr auto value = (CanCompare<T, Types>::value && ...);
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
}
