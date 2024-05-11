#include "assertions_console.h"
#include "console.h"

#ifdef __cplusplus
extern "C" {
#endif

// builds the error string
bool assertPrint( const char *expr,
	const char *file,
	const int line,
	const char *function,
	const char *msg )
{
	KeyConsole &console = KeyConsole::getInstance();
	console.error( ">> Assertion failed @ file: ");
	console.error( file );
	console.error( "\n>> function: " );
	console.error( function );
	console.error( "\n>> line: " );
	console.error( std::to_string( line ) );
	console.error( "\n>> ( " );
	console.error( expr );
	console.error( " )\n" );
	if ( msg != nullptr )
	{
		console.error( ">> Diagnostic message: " );
		console.error( msg );
		console.error( "\n" );
	}
	return true;
}

#ifdef __cplusplus
}
#endif