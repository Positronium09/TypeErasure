export module TypeErasure:Features;

import std;

import :Erase;

export namespace TypeErasure
{
	template <FeatureType... Features>
	struct FeatureComposer
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = ValidateType<T, Features...>;
		};

		template <typename V>
		using VTable = VtableComposer<V, Features...>::Type;

		template <typename M>
		using Model = ModelComposer<M, Features...>::Type;

		template <typename I>
		using Interface = InterfaceComposer<I, Features...>::Type;
	};

	/*
	template <FeatureType Feature>
	struct FeatureCombiner<Feature>
	{
		template <typename V>
		using VTable = Feature::template VTable<V>;

		template <typename M>
		using Model = Feature::template Model<M>;

		template <typename I>
		using Interface = Feature::template Interface<I>;
	};

	template <FeatureType... Others>
	struct FeatureCombiner<Others...>
	{
		template <typename V>
		using VTable = VtableComposer<V, Others...>;

		template <typename M>
		using Model = ModelComposer<M, Others...>;

		template <typename I>
		using Interface = InterfaceComposer<I, Others...>;
	};
	*/

	class OutStreamable
	{
		template <typename T>
		struct CanStream
		{
			static constexpr auto value = requires (std::ostream & o, T a)
			{
				o << a;
			};
		};
		template <typename T>
		struct CanWStream
		{
			static constexpr auto value = requires (std::wostream & o, T a)
			{
				o << a;
			};
		};

		public:
		template <typename T>
		struct Validator
		{
			static constexpr auto value = CanStream<T>::value || CanWStream<T>::value;
		};

		template <typename V>
		struct VTable : virtual V
		{
			virtual auto StreamTo(std::ostream&) -> void = 0;
			virtual auto StreamTo(std::wostream&) -> void = 0;
		};

		template <typename M>
		struct Model : M
		{
			using M::M;
			auto StreamTo(std::ostream& os) -> void override
			{
				if constexpr (CanStream<typename M::ObjectType>::value)
				{
					os << this->GetObject();
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
			auto StreamTo(std::wostream& os) -> void override
			{
				if constexpr (CanWStream<typename M::ObjectType>::value)
				{
					os << this->GetObject();
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
		};

		template <typename I>
		struct Interface : I
		{
			using I::I;

			auto StreamTo(std::ostream& os) const -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamTo(os);
			}
			auto StreamTo(std::wostream& os) const -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamTo(os);
			}

			friend auto operator<<(std::ostream& os, const Interface& obj) -> std::ostream&
			{
				obj.StreamTo(os);
				return os;
			}
			friend auto operator<<(std::wostream& os, const Interface& obj) -> std::wostream&
			{
				obj.StreamTo(os);
				return os;
			}
		};
	};

	class InStreamable
	{
		template <typename T>
		struct CanStream
		{
			static constexpr auto value = requires (std::istream & i, T & a)
			{
				i >> a;
			};
		};
		template <typename T>
		struct CanWStream
		{
			static constexpr auto value = requires (std::wistream & i, T & a)
			{
				i >> a;
			};
		};

		public:
		template <typename T>
		struct Validator
		{
			static constexpr auto value = CanStream<T>::value || CanWStream<T>::value;
		};

		template <typename V>
		struct VTable : virtual V
		{
			virtual auto StreamFrom(std::istream&) -> void = 0;
			virtual auto StreamFrom(std::wistream&) -> void = 0;
		};

		template <typename M>
		struct Model : M
		{
			using M::M;
			auto StreamFrom(std::istream& is) -> void override
			{
				if constexpr (CanStream<typename M::ObjectType>::value)
				{
					is >> this->GetObject();
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
			auto StreamFrom(std::wistream& is) -> void override
			{
				if constexpr (CanWStream<typename M::ObjectType>::value)
				{
					is >> this->GetObject();
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
		};

		template <typename I>
		struct Interface : I
		{
			using I::I;
			auto StreamFrom(std::istream& is) -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamFrom(is);
			}
			auto StreamFrom(std::wistream& is) -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamFrom(is);
			}

			friend auto operator>>(std::istream& is, Interface& obj) -> std::istream&
			{
				obj.StreamFrom(is);
				return is;
			}
			friend auto operator>>(std::wistream& is, Interface& obj) -> std::wistream&
			{
				obj.StreamFrom(is);
				return is;
			}
		};
	};

	using Streamable = FeatureComposer<OutStreamable, InStreamable>;
}
