#if defined _DEBUG && !defined NDEBUG
#	include "leak_checker_console.h"
#	include "winner.h"
#	include "assertions_console.h"
#	include "console.h"


namespace debugLeak
{

_CrtMemState g_start, g_end, g_diff;


bool anyMemoryLeaks()
{
	_CrtMemCheckpoint( &g_end ); 

	// confirms integrity of memory blocks allocated by debug heap
	ASSERT( _CrtCheckMemory() == TRUE, "\n_CrtCheckMemory() assertion failed\n" );

	int diff = _CrtMemDifference( &g_diff, &g_start, &g_end );
	if ( diff > 0 )
	{
		const std::wstring msg = L"\n_CrtMemDifference output = " + std::to_wstring( diff );
		OutputDebugStringW( msg.c_str() );
		_CrtMemDumpStatistics( &g_diff );
	}

	if ( _CrtDumpMemoryLeaks() )	// prints to output window
	{
		return true;
	}
	else
	{
		_CrtMemCheckpoint( &g_start );	// if no leaks up this point then mark now as the new start of checking for memory leaks
		return false;
	}
}

LeakChecker::LeakChecker()
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_REPORT_FLAG | _CRTDBG_LEAK_CHECK_DF );
	_CrtMemCheckpoint( &g_start );

	KeyConsole &console = KeyConsole::getInstance();
	console.log( "\nMemory leak checker setup\n" );
	OutputDebugStringW( L"Memory leak checker setup complete.\n" );
}

LeakChecker::~LeakChecker() noexcept
{
	if ( anyMemoryLeaks() )
	{
		OutputDebugStringW( L"\nLeaking..\n" );
	}
	else
	{
		OutputDebugStringW( L"\nNo leaks. : )\n" );
	}
}


LeakChecker g_leakChecker{};


};// namespace debugLeak


#endif	// _DEBUG