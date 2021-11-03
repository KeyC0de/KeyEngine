#include "os_utils.h"
#include "utils.h"
#include "assertions_console.h"
#include <fstream>
#include <thread>


namespace util
{


std::string printHresultErrorDescription( HRESULT hres )
{
	_com_error error{hres};
	return util::ws2s( error.ErrorMessage() );
}

std::wstring printHresultErrorDescriptionW( HRESULT hres )
{
	_com_error error{hres};
	return error.ErrorMessage();
}

std::string getLastErrorAsString()
{
	// get the error message, if any
	DWORD errorMsgId = ::GetLastError();
	if ( errorMsgId == 0 )
	{
		return "";
	}

	LPSTR buff = nullptr;
	size_t messageLength = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMsgId,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		(LPSTR)&buff,
		0,
		nullptr );

	std::string message;
	message.assign( buff, messageLength );
	// free the buffer allocated by the system
	LocalFree( buff );
	return message;
}

std::string getLastNtErrorAsString( DWORD ntStatusCode )
{
	LPSTR ntStatusMessage = nullptr;
	HMODULE hNtdll = LoadLibraryA( "ntdll.dll" );
	
	size_t messageLength = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_FROM_HMODULE,
		hNtdll,
		ntStatusCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR) &ntStatusMessage,
		0,
		nullptr );

	std::string message;
	message.assign( ntStatusMessage,
		messageLength );
	// free the buffer allocated by the system
	LocalFree( ntStatusMessage );
	FreeLibrary( hNtdll );
	return message;
}

std::wstring bstrToStr( const BSTR& bstr )
{
	ASSERT( bstr != nullptr, "BSTR was null!" );
	std::wstring str{bstr, SysStringLen( bstr )};	// takes ownership so no need to SysFreeString
	return str;
}

#pragma warning( disable : 4267 )
BSTR strToBstr( const std::wstring& str )
{
	ASSERT( !str.empty(), "String was null!" );
	BSTR bstr = SysAllocStringLen( str.data(),
		str.size() );
	return bstr;
}
#pragma warning( default : 4267 )

__int64 filetimeToInt64( const FILETIME& fileTime )
{
	ULARGE_INTEGER ui64;
	ui64.LowPart = fileTime.dwLowDateTime;
	ui64.HighPart = fileTime.dwHighDateTime;
	return static_cast<__int64>( ui64.QuadPart );
}

void pinThreadToCore( HANDLE hThread,
	DWORD core )
{
	// a set bit represents a CPU core
	DWORD_PTR mask = ( static_cast<DWORD_PTR>( 1 ) << core );
	auto ret = SetThreadAffinityMask( GetCurrentThread(),
		mask );
}

static std::vector<HANDLE> g_detachedThreads;

void setupDetachedThreadsVector( unsigned nThreads )
{
	g_detachedThreads.reserve( nThreads );
}

void terminateDetachedThreads()
{
#if defined _DEBUG && !defined NDEBUG
	KeyConsole& console = KeyConsole::getInstance();
	console.print( "Clearing up detached threads\n" );
#endif
	for ( const auto th : g_detachedThreads )
	{
		DWORD exitCode;
		int ret;
		HRESULT hres;
		ret = GetExitCodeThread( th,
				&exitCode );
		ASSERT_HRES_WIN32_IF_FAILED( hres );

		ret = TerminateThread( th,
			exitCode );
		ASSERT_HRES_WIN32_IF_FAILED( hres );
	}
}

//===================================================
//	\function	doPeriodically
//	\brief  like a timer event
//			executes void(*f)() function at periodic (ms) intervals
//	\date	2021/09/06 1:05
void doPeriodically( const std::function<void(void)>& f,
	size_t intervalMs,
	bool now )
{
	std::thread t{[f, intervalMs, now] () -> void
		{
			if ( now )
			{
				while ( true )
				{
					f();
					auto chronoInterval = std::chrono::milliseconds( intervalMs );
					std::this_thread::sleep_for( chronoInterval );
				}
			}
			else
			{
				while ( true )
				{
					auto chronoInterval = std::chrono::milliseconds( intervalMs );
					std::this_thread::sleep_for( chronoInterval );
					f();
				}
			}
		}
	};
	g_detachedThreads.push_back( t.native_handle() );
	t.detach();
}

void doAfter( const std::function<void(void)>& f,
	size_t intervalMs )
{
	std::thread t{[f, intervalMs] () -> void
		{
			auto chronoInterval = std::chrono::milliseconds( intervalMs );
			std::this_thread::sleep_for( chronoInterval );
			f();
		}
	};
	g_detachedThreads.push_back( t.native_handle() );
	t.detach();
}

std::optional<DWORD> registryGetDword( HKEY hKey,
	const std::wstring& regName )
{
	DWORD bufferSize = sizeof( DWORD );
	DWORD val = 0ul;
	long ret = RegQueryValueExW( hKey,
		regName.c_str(),
		nullptr,
		nullptr,
		reinterpret_cast<LPBYTE>( &val ),
		&bufferSize );
	if ( ret != ERROR_SUCCESS )
	{
		return std::nullopt;
	}
	return val;
}

std::optional<std::wstring> registryGetString( HKEY hKey,
	const std::wstring& regName )
{
	wchar_t buffer[512];
	DWORD bufferSize = sizeof( buffer );
	long ret = RegQueryValueExW( hKey,
		regName.c_str(),
		nullptr,
		nullptr,
		reinterpret_cast<LPBYTE>( buffer ),
		&bufferSize );
	if ( ret != ERROR_SUCCESS )
	{
		return std::nullopt;
	}
	std::wstring str{std::begin( buffer ), std::end( buffer )};
	return str;
}


}// namespace util