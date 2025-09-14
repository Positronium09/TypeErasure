import std;

import TypeErasure;

using namespace TypeErasure;

auto main() -> int
{
	auto x = 42;
	auto any = MakeAny<Streamable>(x);
	std::cout << any << '\n';
	std::cin >> any;
	std::cout << any << '\n';
	std::cout << x << '\n';

	return 0;
}
