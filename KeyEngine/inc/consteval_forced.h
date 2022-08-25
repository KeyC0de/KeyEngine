#pragma once


// Goal: Making functions guaranteed to be executed at compile time
// One way to make a variable guaranteed to be compiled at runtime is to make it call a constexpr function.
template<auto V>
static constexpr auto constevalForced = V;
// you would do now:
// consteval_forced<functionName(parameters...)>;
// this is a bit verbose
// so instead for each function you want to guarantee compile time evaluation do:
//#define CONSTEVAL_FUNCTION_NAME(...) constevalForced<functionName(__VA_ARGS__)>

constexpr decltype( auto ) calculateAddition( int x,
	int y )
{
	return x + y;
}
#define CONSTEVAL_CALC_ADDITION( ... ) constevalForced<calculateAddition( __VA_ARGS__ )>

/*
auto result = CONSTEVAL_CALC_ADDITION( 5,
		7 );
std::cout << result << '\n';
*/