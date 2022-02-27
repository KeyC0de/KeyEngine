#include "winner.h"
#include "game.h"
#include "utils.h"
#include "console.h"
#include "sysmetrics.h"
#include <eh.h>
#include <exception>
#include <iostream>
#include "signal_handling.h"
#include <tuple>

#ifndef NO_DUMPS
#	include "dumpling.h"
static wchar_t g_dumpFile[MAX_PATH];
static bool g_windowsExceptionOccurred = false;
#endif // NO_DUMPS

#define KEY_EXCEPTION_EXIT			-1
#define STD_EXCEPTION_EXIT			-2
#define UNKNOWN_EXCEPTION_EXIT		-3
#define SEH_EXCEPTION_EXIT			-4


std::tuple<int,int> parseCommandLineArguments();
int runWindowsProgram( _In_ wchar_t* pCmdLine );
void firstly();
void finally();

int WINAPI wWinMain( _In_ HINSTANCE hinstance,
	_In_opt_ HINSTANCE hprevInstance,
	_In_ wchar_t* pCmdLine,
	_In_ int nShowCmd )
{
	(void)hprevInstance;
	int exitCode;
	__try
	{
		firstly();
		exitCode = runWindowsProgram( pCmdLine );
	}
	__finally
	{
		finally();
	}

#if defined _DEBUG && !defined NDEBUG
	while ( !getchar() );
#endif
	return exitCode;
}

int runWindowsProgram( _In_ wchar_t* pCmdLine )
{
	auto runProgram = [&pCmdLine] () -> int
	{
		try
		{
			std::string commandLine = util::ws2s( pCmdLine );
#if defined _DEBUG && !defined NDEBUG
			KeyConsole& console = KeyConsole::getInstance();
			windowsMetricsCheckTest();
#endif
			/// Make sure to set the GraphicsMode in graphics_mode.h
			const auto& [width, height] = parseCommandLineArguments();
			Sandbox3d game{width, height};
			//Arkanoid game{800, 600};
			//Snake game{800, 600};
			return game.loop();
		}
		catch ( const KeyException& ex )
		{
			MessageBoxW( nullptr,
				util::s2ws( ex.what() ).data(),
				util::s2ws( ex.getType() ).data(),
				MB_OK | MB_ICONEXCLAMATION );
			return KEY_EXCEPTION_EXIT;
		}
		catch ( const std::exception& ex )
		{
			MessageBoxW( nullptr,
				util::s2ws( ex.what() ).data(),
				L"Standard Exception",
				MB_OK | MB_ICONEXCLAMATION );
			return STD_EXCEPTION_EXIT;
		}
		catch ( ... )
		{
			MessageBoxW( nullptr,
				L"No details available.\nTERMINATING with extreme prejudice.",
				L"Unknown Exception",
				MB_OK | MB_ICONEXCLAMATION );
			return UNKNOWN_EXCEPTION_EXIT;
		}
	};

#ifndef NO_DUMPS
	__try
	{
#endif // !NO_DUMPS
		return runProgram();
#ifndef NO_DUMPS
	}
	__except( generateDump( GetExceptionInformation(), g_dumpFile ) )
	{
		g_windowsExceptionOccurred = true;
		MessageBoxW( nullptr,
			L"Dump generated",
			L"Windows SEH Exception!",
			MB_ICONERROR | MB_OK );
		return SEH_EXCEPTION_EXIT;
	}
#endif // !NO_DUMPS
}
//////////////////////////////////////////////////////////////////////

std::tuple<int,int> parseCommandLineArguments()
{
	HRESULT hres;
	LPWSTR commandLine = GetCommandLineW();
	int argc;
	LPWSTR* argv = CommandLineToArgvW( commandLine,
		&argc );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

	wchar_t* end;
	int width = std::wcstol( argv[1],
		&end,
		10 );
	int height = std::wcstol( argv[2],
		&end,
		10 );
#if defined _DEBUG && !defined NDEBUG
	KeyConsole& console = KeyConsole::getInstance();
	console.print( "(width,height)=(" + std::to_string( width ) + "," + std::to_string( height ) + ")" );
#endif
	return {width, height};
}

void firstly()
{
	fflush( stdin );
	fflush( stderr );
	fflush( stdout );

	std::set_terminate( []()
	{
		OutputDebugStringW( L"Unhandled exception. Aborting..\n" );
		std::cout << "KeyEngine Unhandled exception: abort()ing..\n";
		std::abort();
	});

	//std::atexit( util::terminateDetachedThreads );
	std::signal( SIGINT,
		installSigintHandler );
	//util::setupDetachedThreadsVector( std::thread::hardware_concurrency() );
}

void finally()
{
#if defined _DEBUG && !defined NDEBUG
	KeyConsole& console = KeyConsole::getInstance();
	using namespace std::string_literals;
#	ifndef NO_DUMPS
	if ( g_windowsExceptionOccurred )
	{
		const std::string& dumpLog = "\nMiniDump Location = "s
			+ util::ws2s( g_dumpFile )
			+ "\n"s;
		console.log( dumpLog );
		OutputDebugStringA( dumpLog.c_str() );
	}
#	endif // NO_DUMPS
	console.log( "KeyEngine shutting down..\n"s );
	console.log( "Shutting down console\n"s );
	console.resetInstance();
#endif
}