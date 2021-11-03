#include "assertions_console.h"
#include "console.h"

#if defined _DEBUG && !defined NDEBUG

#ifdef __cplusplus
extern "C" {
#endif

// builds the error string
bool assertPrint( const char* expr,
	const char* file,
	int line,
	const char* function,
	const char* msg )
{
	KeyConsole& console = KeyConsole::getInstance();
	console.log( ">> Assertion failed @ file: ");
	console.log( file );
	console.log( "\n>> function: " );
	console.log( function );
	console.log( "\n>> line: " );
	console.log( std::to_string( line ) );
	console.log( "\n>> !( " );
	console.log( expr );
	console.log( " )\n" );
	if ( msg != nullptr )
	{
		console.log( ">> Diagnostic message: " );
		console.log( msg );
		console.log( "\n" );
		return true;
	}
	return true;
}

#ifdef __cplusplus
}
#endif

#endif // _DEBUG