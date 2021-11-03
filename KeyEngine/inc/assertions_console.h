#pragma once

#if defined _DEBUG && !defined NDEBUG

#include <iostream>

#	ifdef __cplusplus
extern "C" {
#	endif

//#		define LSTR( expr ) L ## expr
//#		define STRINGIFY( expr ) LSTR( #expr )
#	define STRINGIFY( expr ) #expr
extern bool assertPrint( const char* expr,
	const char* file,
	int line,
	const char* function,
	const char* msg = "" );

// assert that arg is true, if not print error
#	define ASSERT( arg, msg ) if ( !(arg) && assertPrint( STRINGIFY( arg ),\
				__FILE__,\
				__LINE__,\
				__FUNCTION__,\
				msg ) )\
				{\
					__debugbreak();\
					std::exit( -1 );\
				}

// assert with the optional argument not supplied
#	define ASSERTO( arg ) if ( !(arg) && assertPrint( STRINGIFY( arg ),\
				__FILE__,\
				__LINE__,\
				__FUNCTION__ ) )\
				{\
					__debugbreak();\
					std::exit( -1 );\
				}

#	ifdef __cplusplus
}
#	endif

#else
#	define ASSERT( arg, msg ) void(0);
#endif // _DEBUG