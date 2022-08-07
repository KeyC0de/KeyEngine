#include <cstdlib>
#include <string>
#include "signal_handling.h"
#include "console.h"


void installSigintHandler( int signum )
{
	if ( signum == SIGINT )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "External user-initiated interrupt!\n"s );
		std::abort();
	}
}

void installSigsegvHandler( int signum )
{
	if ( signum == SIGSEGV )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Segmentation Fault!\n"s );
		std::abort();
	}
}

void installSigtermHandler( int signum )
{
	if ( signum == SIGTERM )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Termination Request Interrupt sent to the program!\n"s );
		std::abort();
	}
}

void installSigillHandler( int signum )
{
	if ( signum == SIGILL )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Invalid Instruction interrupt!\n"s );
		std::abort();
	}
}

void installSigabrtHandler( int signum )
{
	if ( signum == SIGABRT )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Aborting...\n"s );
		std::exit( -987654321 );
	}
}

void installSigfpeHandler( int signum )
{
	if ( signum == SIGFPE )
	{
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Erroneous arithmetic operation interrupt!\n"s );
		std::abort();
	}
}