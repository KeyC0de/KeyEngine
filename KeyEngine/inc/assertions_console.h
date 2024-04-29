#pragma once

#include <iostream>

#define LSTR( expr ) L ## expr
#define STRINGIFYW( expr ) LSTR( #expr )
#define STRINGIFY( expr ) #expr

#ifdef __cplusplus
extern "C" {
#endif
extern bool assertPrint( const char *expr, const char *file, const int line, const char *function, const char *msg = "" );
#ifdef __cplusplus
}
#endif

#if defined _DEBUG && !defined NDEBUG
// assert that arg is true, if not print error
#	define ASSERT( arg, msg ) if ( !(arg) && assertPrint( STRINGIFY( arg ), __FILE__, __LINE__, __FUNCTION__, msg ) )\
				{\
					__debugbreak();\
					std::exit( -1 );\
				}

// assert with the optional argument not supplied
#	define ASSERTO( arg ) if ( !(arg) && assertPrint( STRINGIFY( arg ), __FILE__, __LINE__, __FUNCTION__ ) )\
				{\
					__debugbreak();\
					std::exit( -1 );\
				}
#else
#	define ASSERT( arg, msg ) void(0);
#endif // _DEBUG