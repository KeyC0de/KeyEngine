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
	FreeLibrary( hNtdll );
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

static std::vector<HANDLE> g_detachedThreads;

void setupDetachedThreadsVector( unsigned nThreads )
{
	g_detachedThreads.reserve( nThreads );
}

void terminateDetachedThreads()
{
#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::instance();
	console.print( "Clearing up detached threads\n" );
#endif
	for ( const auto th : g_detachedThreads )
	{
		DWORD exitCode;
		int ret;
		HRESULT hres = 0;
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
	const size_t intervalMs,
	const bool now )
{
	std::thread t{[f, intervalMs, now] () -> void
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
	};
	g_detachedThreads.push_back( t.native_handle() );
	t.detach();
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
	THREADENTRY32 te;
	te.dwSize = sizeof( te );
	DWORD current_pid = GetCurrentProcessId();
	DWORD current_tid = GetCurrentThreadId();

	Thread32First( thread_enumerator, &te );
	do
	{
		if ( te.th32OwnerProcessID != current_pid ) continue;
		if ( te.th32ThreadID == current_tid ) continue;

		HANDLE th = OpenThread( THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, false, te.th32ThreadID );
		SuspendThread( th );
	} while ( Thread32Next( thread_enumerator, &te ) );
	CloseHandle( thread_enumerator );
}

hint std::size_t* getUniqueMemory()
{
	return reinterpret_cast<std::size_t*>( new std::size_t{0} );
}

}// namespace util