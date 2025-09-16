import std;

import TypeErasure;

using namespace TypeErasure;
using namespace TypeErasure::Features;

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

	auto operator==(const Point& other) const -> bool
	{
		return x == other.x && y == other.y;
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

	friend auto operator<<(std::ostream& os, const Triangle& t) -> std::ostream&
	{
		os << "Triangle(";
		os << "A: (" << t.a.x << ", " << t.a.y << "), ";
		os << "B: (" << t.b.x << ", " << t.b.y << "), ";
		os << "C: (" << t.c.x << ", " << t.c.y << ")";
		os << ")";
		return os;
	}

	friend auto operator<<(std::wostream& os, const Triangle& t) -> std::wostream&
	{
		os << L"Triangle(";
		os << L"A: (" << t.a.x << L", " << t.a.y << L"), ";
		os << L"B: (" << t.b.x << L", " << t.b.y << L"), ";
		os << L"C: (" << t.c.x << L", " << t.c.y << L")";
		os << L")";
		return os;
	}
};

template <typename CharT>
struct std::formatter<Point, CharT>
{
	template <typename FormatParseContext>
	static constexpr auto parse(FormatParseContext& ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	static auto format(const Point& p, FormatContext& ctx)
	{
		return std::format_to(ctx.out(), "Point({}, {})", p.x, p.y);
	}
};

auto main() -> int
{
	auto x = 42;
	Triangle t;
	const auto any = MakeAnyCRef<Drawable, OutStreamable>(t);
	const auto any2 = MakeAnyRef<Drawable, Formattable>(Point{ 5, 5 });
	std::println("{}", any);
	std::println("{}", any2);
	std::cout << any << '\n';
	std::wcout << any << L'\n';
	any.Draw();
	any2.Draw();

	const auto comp1 = MakeAny<EqualityComparable, EqualityComparableWith<float>>(55);
	const auto comp2 = MakeAny<EqualityComparable>(55);
	const auto comp3 = MakeAny<EqualityComparable>(57);
	std::println("comp1 == comp2: {}", comp1 == comp2);
	std::println("comp1 != comp3: {}", comp1 != comp3);
	std::println("{}", comp1.GetObject<int>());
	std::println("{}", comp2.GetObject<int>());
	std::println("{}", comp3.GetObject<int>());
	std::println("comp1 == 55.0F: {}", comp1 == 55.0F);

	return 0;
}
