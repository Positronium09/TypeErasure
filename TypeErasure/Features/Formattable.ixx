export module TypeErasure:Features.Formattable;

import std;

import :Erase;

export namespace TypeErasure::Features
{
	struct Formattable
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = std::formattable<T, char> || std::formattable<T, wchar_t>;
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Format(std::format_context& ctx) const -> decltype(ctx.out()) = 0;
			virtual auto Format(std::wformat_context& ctx) const -> decltype(ctx.out()) = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Format(std::format_context& ctx) const -> decltype(ctx.out()) override
			{
				if constexpr (std::formattable<typename M::ObjectType, char>)
				{
					return std::format_to(ctx.out(), "{}", this->GetObject());
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
			auto Format(std::wformat_context& ctx) const -> decltype(ctx.out()) override
			{
				if constexpr (std::formattable<typename M::ObjectType, wchar_t>)
				{
					return std::format_to(ctx.out(), L"{}", this->GetObject());
				}
				else
				{
					throw std::bad_any_cast{ };
				}
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto Format(std::format_context& ctx) const -> decltype(ctx.out())
			{
				if (!this->HasValue())
				{
					throw std::bad_any_cast{ };
				}
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->Format(ctx);
			}
			auto Format(std::wformat_context& ctx) const -> decltype(ctx.out())
			{
				if (!this->HasValue())
				{
					throw std::bad_any_cast{ };
				}
				return dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->Format(ctx);
			}
		};
	};
}

export template <typename CharT, TypeErasure::FeatureType... Features>
struct std::formatter<TypeErasure::Any<Features...>, CharT>
{
	template <typename FormatParseContext>
	static constexpr auto parse(FormatParseContext& ctx)
	{
		if (ctx.begin() != ctx.end() && *ctx.begin() != '}')
		{
			throw std::format_error{ "Format arguments for Any is not supported" };
		}

		return ctx.begin();
	}

	template <typename FormatContext>
	static auto format(const TypeErasure::Any<Features...>& any, FormatContext& ctx)
	{
		if constexpr (any.template HasFeature<TypeErasure::Features::Formattable>())
		{
			return any.Format(ctx);
		}
		else
		{
			if (any.HasValue())
			{
				return std::format_to(ctx.out(), "Any[{}]", any.GetTypeInformation().type.name());
			}
			return std::format_to(ctx.out(), "Any[empty={}]", !any.HasValue());
		}
	}
};
