import std;

import TypeErasure;

using namespace TypeErasure;

struct Drawable
{
	template <typename T>
	struct Validator
	{
		static constexpr auto value = requires(const T& a)
		{
			a.Draw();
		};
	};

	template <typename V>
	struct VTable : virtual V
	{
		virtual auto Draw() const -> void = 0;
	};

	template <typename M>
	struct Model : M
	{
		using M::M;
		auto Draw() const -> void override
		{
			this->GetObject().Draw();
		}
	};

	template <typename I>
	struct Interface : virtual I
	{
		auto Draw() const -> void
		{
			if (!this->HasValue())
			{
				throw std::bad_any_cast{ };
			}
			dynamic_cast<VTable<VTableBase>*>(this->GetVTable())->Draw();
		}
	};
};

struct Point
{
	int x;
	int y;

	Point(const int x, const int y) :
		x{ x }, y{ y }
	{ };

	auto Draw() const -> void
	{
		std::cout << "Point(" << x << ", " << y << ")\n";
	}
};

struct Triangle
{
	Point a;
	Point b;
	Point c;

	Triangle() :
		a{ 0, 0 },
		b{ 1, 0 },
		c{ 0, 1 }
	{ };

	auto Draw() const -> void
	{
		std::cout << "Triangle(\n";
		a.Draw();
		b.Draw();
		c.Draw();
		std::cout << ")\n";
	}
};

auto main() -> int
{
	auto x = 42;
	Triangle t;
	const auto any = MakeAnyCRef<Drawable>(t);
	const auto any2 = MakeAnyRef<Drawable>(Point{ 5, 5 });
	any.Draw();
	any2.Draw();
	std::println("{} {} {}", any.GetTypeInformation().type.name(), any.IsRef(), any.IsCRef());
	// const auto& y = any.GetObject<const int&>();
	//y = 100;
	// std::cout << x << '\n';

	return 0;
}
