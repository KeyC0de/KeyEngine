#include "os_utils.h"
#include "utils.h"
#include "assertions_console.h"
#include <fstream>
#include <thread>
#include "util_exception.h"


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
	size_t messageLength = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorMsgId, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&buff, 0, nullptr );

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

	size_t messageLength = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE, hNtdll, ntStatusCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &ntStatusMessage, 0, nullptr );

	std::string message;
	message.assign( ntStatusMessage, messageLength );
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
	BSTR bstr = SysAllocStringLen( str.data(), str.size() );
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
	auto ret = SetThreadAffinityMask( GetCurrentThread(), mask );
}

void setCurrentThreadName( const std::string &name )
{
	// Thread objects in the Windows OS do not have a name. Naming threads is purely a feature of the debugger.
	// The function to 'set' a thread's name is SetThreadName and it raises an MS_VC_EXCEPTION that's caught by the debugger, and the information used for the debugging experience. If no debugger is present, the exception filter simply continues execution. Unless you attach a debugger the function call will fail.
	// The description of a thread can be set more than once; the most recently set value is used. You can retrieve the description of a thread by calling GetThreadDescription.
	HRESULT hres = SetThreadDescription( GetCurrentThread(), L"ThisIsMyThreadName!" );
	ASSERT_HRES_IF_FAILED( hres );
}

std::string getKnownFolderPath( const int id /*= CSIDL_PERSONAL*/ )
{
	std::wstring myDocuments;
	myDocuments.reserve( 100 );
	HRESULT result = SHGetFolderPathW( nullptr, CSIDL_PERSONAL/*FOLDERID_AppDataDocuments*/, nullptr, SHGFP_TYPE_CURRENT, myDocuments.data() );

	if ( result != S_OK )
	{
		THROW_UTIL_EXCEPTION( "Could not find the known document folder!" );
	}

	return util::ws2s( myDocuments );
}

#pragma region DetachedThreads
// A global event all threads can reach
static HANDLE g_hThreadQuitcEvent;
static std::vector<HANDLE> g_detachedThreads;

void setupDetachedThreadsVector( unsigned nThreads )
{
	g_hThreadQuitcEvent = CreateEventW( nullptr, TRUE, FALSE, nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;
	g_detachedThreads.reserve( nThreads );
}

void terminateDetachedThreads()
{
#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::getInstance();
	console.print( "Clearing up detached threads\n" );
#endif
	for ( HANDLE hThread : g_detachedThreads )
	{
		DWORD exitCode = 0;
		/// WARNING: There's no way to cleanly terminate a detached thread. Join them instead in main, or use a thread pool.
		// Terminate WinAPI thread (without using TerminateThread which is extremely error-prone, brute force approach and doesn't allow proper thread cleanup)
		// tell thread to stop
		SetEvent( g_hThreadQuitcEvent );
		ASSERT_HRES_WIN32_IF_FAILED;

		// now wait for thread to signal termination
		WaitForSingleObject( hThread, INFINITE );

		//TerminateThread( hThread, exitCode );
		ASSERT_HRES_WIN32_IF_FAILED;

		GetExitCodeThread( hThread, &exitCode );
		ASSERT_HRES_WIN32_IF_FAILED;

		CloseHandle( hThread );
		ASSERT_HRES_WIN32_IF_FAILED;
	}
	CloseHandle( g_hThreadQuitcEvent );
	ASSERT_HRES_WIN32_IF_FAILED;
}
#pragma endregion DetachedThreads


std::optional<DWORD> registryGetDword( const HKEY hKey,
	const std::wstring &regName )
{
	DWORD bufferSize = sizeof( DWORD );
	DWORD val = 0ul;
	long ret = RegQueryValueExW( hKey, regName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>( &val ), &bufferSize );
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
	long ret = RegQueryValueExW( hKey, regName.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>( buffer ), &bufferSize );
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
		if ( te.th32OwnerProcessID != current_pid )
		{
			continue;
		}
		if ( te.th32ThreadID == current_tid )
		{
			continue;
		}

		HANDLE th = OpenThread( THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, false, te.th32ThreadID );
		if ( th != nullptr )
		{
			SuspendThread( th );
		}
	} while ( Thread32Next( thread_enumerator, &te ) );
	CloseHandle( thread_enumerator );
}

void launchProcess( const std::string &path )
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof( STARTUPINFOW ) );
	ZeroMemory( &pi, sizeof( PROCESS_INFORMATION ) );
	si.cb = sizeof( STARTUPINFOW );

	CreateProcessW( util::s2ws( path ).c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi );
	ASSERT_HRES_WIN32_IF_FAILED;

	/*
	std::cout << "Process creation successful.\n";
	std::cout << "Process ID: "
		<< pi.dwProcessId
		<< '\n';
	std::cout << "GetProcessId: "
		<< GetProcessId( pi.hProcess )
		<< '\n';
	std::cout << "Thread ID: "
		<< pi.dwThreadId
		<< '\n';
	std::cout << "GetThreadId: "
		<< GetThreadId( pi.hThread )
		<< '\n';
	*/
}


}// namespace util