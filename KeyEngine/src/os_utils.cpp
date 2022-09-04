#include "os_utils.h"
#include "utils.h"
#include "assertions_console.h"
#include <fstream>
#include <thread>


namespace util
{

std::string printHresultErrorDescription( const HRESULT hres )
{
	_com_error error{hres};
	return util::ws2s( error.ErrorMessage() );
}

std::wstring printHresultErrorDescriptionW( const HRESULT hres )
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

std::string getLastNtErrorAsString( const DWORD ntStatusCode )
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
	if ( hNtdll != nullptr )
	{
		FreeLibrary( hNtdll );
	}
	return message;
}

std::wstring bstrToStr( const BSTR &bstr )
{
	ASSERT( bstr != nullptr, "BSTR was null!" );
	std::wstring str{bstr, SysStringLen( bstr )};	// takes ownership so no need to SysFreeString
	return str;
}

#pragma warning( disable : 4267 )
BSTR strToBstr( const std::wstring &str )
{
	ASSERT( !str.empty(), "String was null!" );
	BSTR bstr = SysAllocStringLen( str.data(),
		str.size() );
	return bstr;
}
#pragma warning( default : 4267 )

__int64 filetimeToInt64( const FILETIME &fileTime )
{
	ULARGE_INTEGER ui64{0ul,0ul};
	ui64.LowPart = fileTime.dwLowDateTime;
	ui64.HighPart = fileTime.dwHighDateTime;
	return static_cast<__int64>( ui64.QuadPart );
}

void pinThreadToCore( const HANDLE hThread,
	const DWORD core )
{
	// a set bit represents a CPU core
	DWORD_PTR mask = ( static_cast<DWORD_PTR>( 1 ) << core );
	auto ret = SetThreadAffinityMask( GetCurrentThread(),
		mask );
}

// A global event all threads can reach
static HANDLE g_hThreadQuitcEvent;
static std::vector<HANDLE> g_detachedThreads;

void setupDetachedThreadsVector( unsigned nThreads )
{
	HRESULT hres = 0;
	g_hThreadQuitcEvent = CreateEventW( nullptr,
		TRUE,
		FALSE,
		nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;
	g_detachedThreads.reserve( nThreads );
}

void terminateDetachedThreads()
{
#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::instance();
	console.print( "Clearing up detached threads\n" );
#endif
	for ( HANDLE hThread : g_detachedThreads )
	{
		DWORD exitCode = 0;
		HRESULT hres = 0;
		/// WARNING: There's no way to cleanly terminate a detached thread. Join them instead in main, or use a thread pool.
		// Terminate WinAPI thread (without using TerminateThread which is extremely error-prone, brute force approach and doesn't allow proper thread cleanup)
		// tell thread to stop
		SetEvent( g_hThreadQuitcEvent );
		ASSERT_HRES_WIN32_IF_FAILED;

		// now wait for thread to signal termination
		WaitForSingleObject( hThread,
			INFINITE );

		//TerminateThread( hThread,
		//	exitCode );
		ASSERT_HRES_WIN32_IF_FAILED;

		GetExitCodeThread( hThread,
				&exitCode );
		ASSERT_HRES_WIN32_IF_FAILED;

		CloseHandle( hThread );
		ASSERT_HRES_WIN32_IF_FAILED;
	}
	HRESULT hres = 0;
	CloseHandle( g_hThreadQuitcEvent );
	ASSERT_HRES_WIN32_IF_FAILED;
}

void doPeriodically( const std::function<void(void)>& f,
	const size_t intervalMs,
	const bool now )
{
	if ( now )
	{
		while ( true )
		{
			f();
			auto chronoInterval = std::chrono::milliseconds( intervalMs );
		}
	}
	else
	{
		while ( true )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( intervalMs ) );
			f();
		}
	}
}

std::optional<DWORD> registryGetDword( const HKEY hKey,
	const std::wstring &regName )
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

std::optional<std::wstring> registryGetString( const HKEY hKey,
	const std::wstring &regName )
{
	wchar_t buffer[512]{0};
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

static void suspendAllThreads()
{
	HANDLE thread_enumerator = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
	THREADENTRY32 te{};
	te.dwSize = sizeof( te );
	DWORD current_pid = GetCurrentProcessId();
	DWORD current_tid = GetCurrentThreadId();

	Thread32First( thread_enumerator, &te );
	do
	{
		if ( te.th32OwnerProcessID != current_pid ) continue;
		if ( te.th32ThreadID == current_tid ) continue;

		HANDLE th = OpenThread( THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, false, te.th32ThreadID );
		if ( th != nullptr )
		{
			SuspendThread( th );
		}
	} while ( Thread32Next( thread_enumerator, &te ) );
	CloseHandle( thread_enumerator );
}

hint std::size_t* getUniqueMemory()
{
	return reinterpret_cast<std::size_t*>( new std::size_t{0} );
}

}// namespace util