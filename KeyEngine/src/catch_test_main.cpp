#define CATCH_CONFIG_MAIN	// catch writes its own main using this define
#include "catch/catch.hpp"
#include <vector>


unsigned int factorial( unsigned int number )
{
	return number <= 1 ?
		number :
		factorial( number - 1 ) * number;
}


// complete ref: https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md#top
/*TEST_CASE( "Testing deposit()" )
{
	BankAccount ba{"testUser"};
	REQUIRE( ba.deposit( 10.0000 ) == true );
}

TEST_CASE( "Testing widthdraw()" )
{
	BankAccount ba{"testUser"};
	REQUIRE( ba.withdraw( 10.0000 ) == false );
}*/

/*
TEST_CASE( "testing display()" )
{
	BankAccount ba{"testUser"};
	REQUIRE( ba.deposit( 50 ) == true );
	REQUIRE( ba.getBalance() == 50 );
	REQUIRE( ba.withdraw( 20 ) == true );
	//REQUIRE( ba.display() == "Your balance is $30." );	// failed
	REQUIRE( ba.display() == "Your balance is $30.000000." );
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
	REQUIRE( factorial( 1 ) == 1 );
	REQUIRE( factorial( 2 ) == 2 );
	REQUIRE( factorial( 3 ) == 6 );
	CHECK( factorial( 10 ) == 3628800 );
	REQUIRE( factorial( 10 ) == 3628800 );

	auto x = GENERATE( range( 1, 11 ) );
	auto y = GENERATE( range( 10, 111 ) );

	CHECK( x < y );

	// CHECK() instead of REQUIRE simply reports the failure and moves on
	// CHECK does not abort like REQUIRE

	std::vector<int> v( 5 );
	REQUIRE( v.size() == 5 );
	REQUIRE( v.capacity() >= 5 );
	// For each SECTION the TEST_CASE is executed from the start. This means that each section is entered with a freshly constructed vector v, that we know has size 5 and capacity at least 5, because the two assertions are also checked before the section is entered. Each run through a test case will execute one, and only one, leaf section.
	// SECTIONs can also be nested.
	SECTION( "reserving smaller does not change size or capacity" )
	{
		v.reserve( 0 );

		REQUIRE( v.size() == 5 );
		REQUIRE( v.capacity() >= 5 );
	}
}

// if all is well console will print:
//"All tests Passed. n assertions in x test cases."

// if something fails it will be reported at the end of the respective test case, like so:
// ... FAILED:
// with expansion:
// ...
*/