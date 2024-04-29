#include <cstdlib>
#include <string>
#include "signal_handling.h"
#include "console.h"


void installSigintHandler( const int signum )
{
	if ( signum == SIGINT )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "External user-initiated interrupt!\n"s );
#endif
		std::abort();
	}
}

void installSigsegvHandler( const int signum )
{
	if ( signum == SIGSEGV )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Segmentation Fault!\n"s );
#endif
		std::abort();
	}
}

void installSigtermHandler( const int signum )
{
	if ( signum == SIGTERM )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Termination Request Interrupt sent to the program!\n"s );
#endif
		std::abort();
	}
}

void installSigillHandler( const int signum )
{
	if ( signum == SIGILL )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Invalid Instruction interrupt!\n"s );
#endif
		std::abort();
	}
}

void installSigabrtHandler( const int signum )
{
	if ( signum == SIGABRT )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Aborting...\n"s );
#endif
		std::exit( -987654321 );
	}
}

void installSigfpeHandler( const int signum )
{
	if ( signum == SIGFPE )
	{
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Erroneous arithmetic operation interrupt!\n"s );
#endif
		std::abort();
	}
}