#pragma once

#if defined _DEBUG && !defined NDEBUG
#	define _CRTDBG_MAP_ALLOC
#	define _CRTDBG_MAP_ALLOC_NEW
#	include <crtdbg.h>
#	include "assertions_console.h"
#	include "winner.h"


namespace debugLeak
{

bool anyMemoryLeaks()
{
	// confirms integrity of memory blocks allocated by debug heap
	ASSERT( _CrtCheckMemory() == TRUE, "_CrtCheckMemory() assertion failed\n" );
	if ( _CrtDumpMemoryLeaks() )
	{
		return true;
	}
	else
	{
		return false;
	}
}

};// namespace debugLeak


#endif