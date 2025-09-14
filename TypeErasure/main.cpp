import std;

import TypeErasure;

using namespace TypeErasure;


struct Test1
{ };

struct Test2
{
	template <typename T>
	struct Validator
	{
		static constexpr auto value = false;
	};
};

struct Test3
{
	template <typename T>
	struct Validator
	{
		static constexpr auto value = true;
	};
};

struct Point
{
	int x = 0;
	int y = 0;
	int z = 0;
	int w = 0;
	int r = 0;
	int q = 0;
	int p = 0;

	friend auto operator<<(std::ostream& os, const Point& pt) -> std::ostream&
	{
		return os << '(' << pt.x << ", " << pt.y << ')';
	}

	friend auto operator>>(std::istream& is, Point& pt) -> std::istream&
	{
		char c;
		return is >> c >> pt.x >> pt.y >> c;
	}
};

auto main() -> int
{
	auto x = 42;
	auto any = MakeAnyRef<Streamable>(Point{ 5, 5 });
	// const auto& y = any.GetObject<const int&>();
	//y = 100;
	std::cout << any << '\n';
	std::cin >> any;
	std::cout << any << '\n';
	// std::cout << x << '\n';

	return 0;
}
