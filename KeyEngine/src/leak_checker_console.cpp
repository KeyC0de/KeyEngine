#if defined _DEBUG && !defined NDEBUG

#	include "leak_checker_console.h"
#	include "console.h"

namespace debugLeak
{

class LeakChecker final
{
public:
	LeakChecker()
	{
		KeyConsole& console = KeyConsole::getInstance();
		console.log( "Memory leak checker setup\n" );
		OutputDebugStringW( L"Memory leak checker setup\n" );
		setupLeakChecker();
	}

	~LeakChecker()
	{
		if ( anyMemoryLeaks() )
		{
			OutputDebugStringW( L"Leaking..\n" );
		}
		else
		{
			OutputDebugStringW( L"No leaks. : )\n" );
		}
	}
	
	static inline void setupLeakChecker()
	{
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF
			| _CRTDBG_REPORT_FLAG
			| _CRTDBG_LEAK_CHECK_DF );
		return;
	}
};

LeakChecker leakChecker{};

};// namespace debugLeak

#endif	// _DEBUG