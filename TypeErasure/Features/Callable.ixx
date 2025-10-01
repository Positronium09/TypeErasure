export module TypeErasure:Features.Callable;

import std;
import :Erase;

export namespace TypeErasure::Features
{
	template <typename>
	struct Callable;

	template <typename Ret, typename... Args>
	struct Callable<Ret(Args...)>
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = std::is_invocable_r_v<Ret, T, Args...>;
		};
		template <typename V>
		struct VTable : virtual V
		{
			virtual auto Call(Args... args) const -> Ret = 0;
		};
		template <typename M>
		struct Model : M
		{
			using M::M;
			auto Call(Args... args) const -> Ret override
			{
				return std::invoke(this->GetObject(), std::forward<Args>(args)...);
			}
		};
		template <typename I>
		struct Interface : virtual I
		{
			auto operator()(Args... args) const -> Ret
			{
				if (!this->HasValue())
				{
					throw std::bad_any_cast{ };
				}

				const auto& vTable = dynamic_cast<const VTable<VTableBase>&>(*this->GetVTable());
				return vTable.Call(std::forward<Args>(args)...);
			}
		};
	};
}
