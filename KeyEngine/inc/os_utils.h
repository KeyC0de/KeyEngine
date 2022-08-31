#pragma once

#include "winner.h"
#include <winternl.h>
#include <comdef.h>
#include <tlhelp32.h>
#include <string>
#include <sstream>
#include "console.h"
#include <functional>
#include <optional>

/*
__declspec(restrict) declares that the return value of a function points to memory that is not aliased. That is, the memory returned by the function is guaranteed to not be accessible through any other pointer in the program.

__declspec(noalias) declares that the function does not modify memory outside the first level of indirection from the function's parameters. That is, the parameters are the only reference to the outside world the function has.

Both of them are just performance hints to the compiler.
*/
#if defined _MSC_VER || defined _WIN32 || defined _WIN64
#	define restricted __declspec( restrict )
#	define noaliasing __declspec( noalias )
#	define hint __declspec( restrict ) __declspec( noalias )
#elif defined __unix__ || defined __unix || defined __APPLE__ && defined __MACH__
#	define restricted __restrict__
#	define noaliasing __restrict__
#	define hint __restrict__
#endif


namespace util
{

//===================================================
//	\function	printHresultErrorDescription
//	\brief  print HRESULT errors in understandable English
//	\date	2021/09/03 21:45
std::string printHresultErrorDescription( HRESULT hres );
std::wstring printHresultErrorDescriptionW( HRESULT hres );
//===================================================
//	\function	getLastErrorAsString
//	\brief  Returns the last Win32 error, in string format.
//			Returns an empty string if there is no error.
//	\date	2020/11/10 1:44
std::string getLastErrorAsString();
std::string getLastNtErrorAsString( DWORD ntStatusCode );

std::wstring bstrToStr( const BSTR &bstr );
BSTR strToBstr( const std::wstring &str );

__int64 filetimeToInt64( const FILETIME &fileTime );
void pinThreadToCore( HANDLE hThread, DWORD core );

void setupDetachedThreadsVector( unsigned nThreads );
void terminateDetachedThreads();
void doPeriodically( const std::function<void(void)>& f, size_t intervalMs, bool now = true );

std::optional<DWORD> registryGetDword( HKEY hKey, const std::wstring &regName );
std::optional<std::wstring> registryGetString( HKEY hKey, const std::wstring &regName );

static void suspendAllThreads();

hint std::size_t* getUniqueMemory();


}// namespace util


#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_RETURN_HRES_IF_FAILED( hres ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s\
			<< __FUNCTION__\
			<< " @ line: "s\
			<< __LINE__\
			<< "\n"s\
			<< util::printHresultErrorDescription( hres )\
			<< "\n\n"s;\
		KeyConsole &console = KeyConsole::getInstance();\
		console.log( oss.str() );\
		__debugbreak();\
		return hres;\
	}
#else
#	define ASSERT_RETURN_HRES_IF_FAILED( hres ) (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_IF_FAILED( hres ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s\
			<< __FUNCTION__\
			<< " @ line: "s\
			<< __LINE__\
			<< "\n"s\
			<< util::printHresultErrorDescription( hres )\
			<< "\n\n"s;\
		KeyConsole &console = KeyConsole::getInstance();\
		console.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_IF_FAILED( hres ) (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_HRES_IF_FAILED if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s\
			<< __FUNCTION__\
			<< " @ line: "s\
			<< __LINE__\
			<< "\n"s\
			<< util::printHresultErrorDescription( hres )\
			<< "\n\n"s;\
		KeyConsole &console = KeyConsole::getInstance();\
		console.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_HRES_IF_FAILED (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_HRES_IF_FAILED_MSG( msg ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s\
			<< __FUNCTION__\
			<< " @ line: "s\
			<< __LINE__\
			<< "\n"s\
			<< util::printHresultErrorDescription( hres )\
			<< "\n"\
			<< "msg: "\
			<< msg\
			<< "\n\n"s;\
		KeyConsole &console = KeyConsole::getInstance();\
		console.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_HRES_IF_FAILED_MSG (void)0;
#endif


#if defined _DEBUG && !defined NDEBUG
// or call getLastErrorAsString()
#	define ASSERT_HRES_WIN32_IF_FAILED( hres ) \
	hres = HRESULT_FROM_WIN32( GetLastError() );\
	ASSERT_HRES_IF_FAILED
#else
#	define ASSERT_HRES_WIN32_IF_FAILED (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_NTSTATUS_IF_FAILED( ntErrorCode ) \
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s\
			<< __FUNCTION__\
			<< " @ line: "s\
			<< __LINE__\
			<< "\n"s\
			<< util::getLastNtErrorAsString( ntErrorCode )\
			<< "\n\n"s;\
		KeyConsole &console = KeyConsole::getInstance();\
		console.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_NTSTATUS_IF_FAILED (void)0;
#endif


#if defined _DEBUG && !defined NDEBUG
// or call getLastErrorAsString()
#	define ASSERT_HRES_REGISTRY_IF_FAILED( ret ) \
	if ( ret != ERROR_SUCCESS )\
	{\
		wchar_t buffer[MAX_PATH];\
		FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM,\
			nullptr,\
			ret,\
			0,\
			buffer,\
			MAX_PATH,\
			nullptr );\
		KeyConsole &console = KeyConsole::getInstance();\
		console.print( util::ws2s( buffer ) );\
		__debugbreak();\
		std::exit( ret );\
	}
#else
#	define ASSERT_HRES_REGISTRY_IF_FAILED (void)0;
#endif