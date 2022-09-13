#include "winner.h"
#include "game.h"
#include "utils.h"
#include "console.h"
#include "graphics_mode.h"
#include "sysmetrics.h"
#include <eh.h>
#include <exception>
#include <iostream>
#include <tuple>
#include "signal_handling.h"
#include "thread_pool_j.h"

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
void firstly();
void finally( const std::exception_ptr &exceptionPtr );


int WINAPI wWinMain( _In_ HINSTANCE hinstance,
	_In_opt_ HINSTANCE hprevInstance,
	_In_ wchar_t *pCmdLine,
	_In_ int nShowCmd )
{
	(void)hprevInstance;
	int exitCode;
	std::exception_ptr exceptionPtr = nullptr;

	auto runProgram = [&pCmdLine, &exceptionPtr] () -> int
	{
		try
		{
			std::string commandLine = util::ws2s( pCmdLine );
#if defined _DEBUG && !defined NDEBUG
			KeyConsole &console = KeyConsole::instance();
			windowsMetricsCheckTest();
#endif
			const auto& [width, height] = parseCommandLineArguments();
			if constexpr ( gph_mode::get() == gph_mode::_3D )
			{
				Sandbox3d game{width, height};
				return game.loop();
			}
			else if constexpr ( gph_mode::get() == gph_mode::_2D )
			{
				SettingsManager &settingsMan = SettingsManager::instance();
				//if ( settingsMan.getSettings().m_game == "Arkanoid2d" )
				//{
					Arkanoid game{800, 600};
					return game.loop();
				//}
				//else if ( settingsMan.getSettings().m_game == "Minesweeper2d" )
				//{
					//Minesweeper game{800, 600};
					//return game.loop();
				//}
			}
		}
		catch ( const KeyException &ex )
		{
			MessageBoxW( nullptr,
				util::s2ws( ex.what() ).data(),
				util::s2ws( ex.getType() ).data(),
				MB_OK | MB_ICONEXCLAMATION );
			return KEY_EXCEPTION_EXIT;
		}
		catch ( const std::exception &ex )
		{
			MessageBoxW( nullptr,
				util::s2ws( ex.what() ).data(),
				L"Standard Exception",
				MB_OK | MB_ICONEXCLAMATION );
			return STD_EXCEPTION_EXIT;
		}
		catch ( ... )
		{
			exceptionPtr = std::current_exception();
			MessageBoxW( nullptr,
				L"No details available.\nTERMINATING with extreme prejudice.",
				L"Unknown Exception",
				MB_OK | MB_ICONEXCLAMATION );
			return UNKNOWN_EXCEPTION_EXIT;
		}
	};

	//__try
	//{
	firstly();
	exitCode = runProgram();
	//}
#ifndef NO_DUMPS
	__except ( generateDump( GetExceptionInformation(), g_dumpFile ) )
	{
		g_windowsExceptionOccurred = true;
		MessageBoxW( nullptr,
			L"Windows SEH Exception!",
			L"Dump generated",
			MB_ICONERROR | MB_OK );

		return SEH_EXCEPTION_EXIT;
	}
#else
	//__finally
	//{
	finally( exceptionPtr );
	//}
#endif // !NO_DUMPS

#if defined _DEBUG && !defined NDEBUG
	while ( !getchar() );
#endif
	return exitCode;
}
//////////////////////////////////////////////////////////////////////

void firstly()
{
	fflush( stdin );
	fflush( stderr );
	fflush( stdout );

	std::set_terminate( [] ()
		{
			OutputDebugStringW( L"Unhandled exception. Aborting..\n" );
			std::cout << "KeyEngine Unhandled exception: abort()ing..\n";
			std::abort();
		} );

	std::signal( SIGINT,
		installSigintHandler );
}

std::tuple<int,int> parseCommandLineArguments()
{
	HRESULT hres = 0;
	LPWSTR commandLine = GetCommandLineW();
	int argc;
	LPWSTR *argv = CommandLineToArgvW( commandLine,
		&argc );
	ASSERT_HRES_WIN32_IF_FAILED;

	wchar_t *end;
	int width = std::wcstol( argv[1],
		&end,
		10 );
	int height = std::wcstol( argv[2],
		&end,
		10 );
#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::instance();
	console.print( "(width,height)=(" + std::to_string( width ) + "," + std::to_string( height ) + ")" );
#endif
	return {width, height};
}

void finally( const std::exception_ptr &exceptionPtr )
{
	ThreadPoolJ::resetInstance();
#if defined _DEBUG && !defined NDEBUG
	KeyConsole &console = KeyConsole::instance();
	using namespace std::string_literals;
#	ifndef NO_DUMPS
	if ( g_windowsExceptionOccurred )
	{
		const std::string &dumpLog = "\nMiniDump Location = "s
			+ util::ws2s( g_dumpFile )
			+ "\n"s;
		console.log( dumpLog );
		OutputDebugStringA( dumpLog.c_str() );
	}
#	endif // NO_DUMPS
#endif	// _DEBUG
	// main() does not catch exceptions thrown from other threads
	// you will need to instrument the threads to catch exceptions and re-throw them
	if ( exceptionPtr )
	{
		try
		{
			std::rethrow_exception( exceptionPtr );
		}
		catch ( const std::exception &ex )
		{
			MessageBoxW( nullptr,
				util::s2ws( ex.what() ).data(),
				L"Thread exited with exception:",
				MB_OK | MB_ICONEXCLAMATION );
		}
	}

#if defined _DEBUG && !defined NDEBUG
	console.log( "KeyEngine shutting down..\n"s );
	console.log( "Shutting down console\n"s );
	console.resetInstance();
#endif	// _DEBUG
}