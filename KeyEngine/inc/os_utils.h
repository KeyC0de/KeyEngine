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
#include <ShlObj.h>
#include <wincred.h>

#define POD_ZERO( arg )		ZeroMemory( &arg, sizeof( arg ) );


namespace util
{

/// \brief	print HRESULT errors in understandable English
std::string printHresultErrorDescription( const HRESULT hres );
std::wstring printHresultErrorDescriptionW( const HRESULT hres );
/// \brief	Returns the last Win32 error, in string format.
//			Returns an empty string if there is no error.
std::string getLastErrorAsString();
std::string getLastNtErrorAsString( const DWORD ntStatusCode );

std::wstring bstrToStr( const BSTR &bstr );
BSTR strToBstr( const std::wstring &str );

__int64 filetimeToInt64( const FILETIME &fileTime );
/// \brief	returns the thread's previous affinity mask
[[maybe_unused]] DWORD_PTR pinThreadToCore( const HANDLE hThread, const DWORD core );
/// \brief Thread objects in the Windows OS do not have a name. Naming threads is purely a feature of the debugger.
/// \brief The function to 'set' a thread's name is SetThreadName and it raises an MS_VC_EXCEPTION that's caught by the debugger, and the information used for the debugging experience. If no debugger is present, the exception filter simply continues execution. Unless you attach a debugger the function call will fail.
/// \brief The description of a thread can be set more than once; the most recently set value is used. You can retrieve the description of a thread by calling GetThreadDescription.
void setCurrentThreadName( const std::string &name );

std::string getKnownFolderPath( const int id = CSIDL_PERSONAL );

#pragma region DetachedThreadsH
void setupDetachedThreadsVector( const unsigned nThreads );
void terminateDetachedThreads();
#pragma endregion DetachedThreadsH

std::optional<DWORD> registryGetDword( const HKEY hKey, const std::wstring &regName );
std::optional<std::wstring> registryGetString( const HKEY hKey, const std::wstring &regName );

static void suspendAllThreads();

void launchProcess( const std::string &path );

template<typename TCallback, typename... TArgs>
decltype( auto ) askForCredentials( TCallback &&f,
	TArgs &&... args )
{
	CREDUI_INFO cui;
	wchar_t pszName[CREDUI_MAX_USERNAME_LENGTH + 1];
	wchar_t pszPwd[CREDUI_MAX_PASSWORD_LENGTH + 1];

	cui.cbSize = sizeof( CREDUI_INFO );
	cui.hwndParent = nullptr;
	// ensure that MessageText and CaptionText identify what credentials to use and which application requires them
	cui.pszMessageText = L"Enter administrator account information";
	cui.pszCaptionText = L"CredUITest";
	cui.hbmBanner = nullptr;
	BOOL fSave = FALSE;
	SecureZeroMemory( pszName, sizeof( pszName ) );
	SecureZeroMemory( pszPwd, sizeof( pszPwd ) );
	unsigned dwErr = CredUIPromptForCredentialsW( &cui,	// CREDUI_INFO structure
		TEXT("TheServer"),					// target for credentials (usually a server)
		nullptr,									// reserved
		0,									// reason
		pszName,								// user name
		CREDUI_MAX_USERNAME_LENGTH + 1,	// max number of chars for user name
		pszPwd,								// password
		CREDUI_MAX_PASSWORD_LENGTH + 1,	// max number of chars for password
		&fSave,										// state of save check box
		CREDUI_FLAGS_GENERIC_CREDENTIALS | CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_DO_NOT_PERSIST );

	auto ret;
	if ( !dwErr )
	{
		//  Put code that uses the credentials here.
		ret = std::invoke( std::forward<TCallback>( f ), std::forward<TArgs>( args ) ... );

		//  when you have finished using the credentials, erase them from memory.
		SecureZeroMemory( pszName, sizeof( pszName ) );
		SecureZeroMemory( pszPwd, sizeof( pszPwd ) );
	}
	return ret;
}


}// namespace util


#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_RETURN_HRES_IF_FAILED( hres ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s << __FUNCTION__ << " @ line: "s << __LINE__ << "\n"s << util::printHresultErrorDescription( hres ) << "\n\n"s;\
		KeyConsole &con = KeyConsole::getInstance();\
		con.log( oss.str() );\
		__debugbreak();\
		return hres;\
	}
#else
#	define ASSERT_RETURN_HRES_IF_FAILED( hres ) (void)hres;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_IF_FAILED( hres ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s << __FUNCTION__ << " @ line: "s << __LINE__ << "\n"s << util::printHresultErrorDescription( hres ) << "\n\n"s;\
		KeyConsole &con = KeyConsole::getInstance();\
		con.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_IF_FAILED( hres ) (void)hres;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_HRES_IF_FAILED if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s << __FUNCTION__ << " @ line: "s << __LINE__ << "\n"s << util::printHresultErrorDescription( hres ) << "\n\n"s;\
		KeyConsole &con = KeyConsole::getInstance();\
		con.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_HRES_IF_FAILED (void)hres;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_HRES_IF_FAILED_MSG( msg ) if ( FAILED( hres ) )\
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s << __FUNCTION__ << " @ line: "s << __LINE__ << "\n"s << util::printHresultErrorDescription( hres ) << "\n" << "msg: " << msg << "\n\n"s;\
		KeyConsole &con = KeyConsole::getInstance();\
		con.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_HRES_IF_FAILED_MSG (void)hres;
#endif


#if defined _DEBUG && !defined NDEBUG
// or call getLastErrorAsString()
#	define ASSERT_HRES_WIN32_IF_FAILED \
	{ \
		HRESULT hres = HRESULT_FROM_WIN32( GetLastError() ); \
		ASSERT_HRES_IF_FAILED; \
	}
#else
#	define ASSERT_HRES_WIN32_IF_FAILED (void)0;
#endif

#if defined _DEBUG && !defined NDEBUG
#	define ASSERT_NTSTATUS_IF_FAILED( ntErrorCode ) \
	{\
		std::ostringstream oss;\
		using namespace std::string_literals;\
		oss	<< "\n"s << __FUNCTION__ << " @ line: "s << __LINE__ << "\n"s << util::getLastNtErrorAsString( ntErrorCode ) << "\n\n"s;\
		KeyConsole &con = KeyConsole::getInstance();\
		con.log( oss.str() );\
		__debugbreak();\
		std::exit( hres );\
	}
#else
#	define ASSERT_NTSTATUS_IF_FAILED (void)hres;
#endif


#if defined _DEBUG && !defined NDEBUG
// or call getLastErrorAsString()
#	define ASSERT_HRES_REGISTRY_IF_FAILED( ret ) \
	if ( ret != ERROR_SUCCESS )\
	{\
		wchar_t buffer[MAX_PATH];\
		FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ret, 0, buffer, MAX_PATH, nullptr );\
		KeyConsole &con = KeyConsole::getInstance();\
		con.print( util::ws2s( buffer ) );\
		__debugbreak();\
		std::exit( ret );\
	}
#else
#	define ASSERT_HRES_REGISTRY_IF_FAILED (void)0;
#endif