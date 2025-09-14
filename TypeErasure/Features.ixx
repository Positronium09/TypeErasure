export module TypeErasure:Features;

import std;

import :Erase;

export namespace TypeErasure
{
	template <FeatureType... Features>
	struct FeatureComposer
	{
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

	struct OutStreamable
	{
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
				os << this->GetObject();
			}
			auto StreamTo(std::wostream& os) -> void override
			{
				os << this->GetObject();
			}
		};

		template <typename I>
		struct Interface : I
		{
			using I::I;

			auto StreamTo(std::ostream& os) -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamTo(os);
			}
			auto StreamTo(std::wostream& os) -> void
			{
				dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->StreamTo(os);
			}

			friend auto operator<<(std::ostream& os, Interface& obj) -> std::ostream&
			{
				obj.StreamTo(os);
				return os;
			}
			friend auto operator<<(std::wostream& os, Interface& obj) -> std::wostream&
			{
				obj.StreamTo(os);
				return os;
			}
		};
	};

	struct InStreamable
	{
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
				is >> this->GetObject();
			}
			auto StreamFrom(std::wistream& is) -> void override
			{
				is >> this->GetObject();
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
