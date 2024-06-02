#include <iostream>
#include "console.h"
#include "key_timer.h"
#include "os_utils.h"
#include "utils.h"
#include "magic_enum/magic_enum.h"

#ifdef NOGDI
# define LF_FACESIZE		32
#define FF_DONTCARE         (0<<4)
#define FW_NORMAL           400
typedef struct _CONSOLE_FONT_INFOEX
{
	ULONG cbSize;
	DWORD nFont;
	COORD dwFontSize;
	UINT  FontFamily;
	UINT  FontWeight;
	WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI SetCurrentConsoleFontEx( HANDLE hConsoleOutput, BOOL bMaximumWindow, PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx );
#ifdef __cplusplus
}
#endif
#endif // NOGDI

#define MAKE_CONSOLE_DEFAULT_TITLE	std::string{s_defaultConsoleTitle} + std::string{" "} + std::string{s_currentVersion}
#define MAX_CONSOLE_TITLE_LEN	128


KeyConsole& KeyConsole::getInstance() noexcept
{
	if ( s_pInstance == nullptr )
	{
		s_pInstance = new KeyConsole;
	}
	return *s_pInstance;
}

void KeyConsole::resetInstance()
{
	if ( s_pInstance != nullptr )
	{
		delete s_pInstance;
	}
}

KeyConsole::KeyConsole( const std::string &fontName )
	:
	m_fp{nullptr},
	m_stdDevice{STD_OUTPUT_HANDLE},
	m_hMode{stdout},
	m_hConsole{GetStdHandle( m_stdDevice )}
{
	fflush( stdout );

	// 1. Allocate console
	if ( !AllocConsole() )
	{
		MessageBoxW( nullptr, util::s2ws( "Unable to create Debug Console." ).data(), util::s2ws( "Notice" ).data(), MB_ICONEXCLAMATION );
	}

	// 2. set console title
	std::string title = MAKE_CONSOLE_DEFAULT_TITLE;
	SetConsoleTitleW( util::s2ws( title ).data() );

	// 3. set the console codepage to UTF-8 UNICODE
	if ( !IsValidCodePage( CP_UTF8 ) )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}
	if ( !SetConsoleCP( CP_UTF8 ) )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}
	if ( !SetConsoleOutputCP( CP_UTF8 ) )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}

	// 4. use a suitable (console) font that supports our desired glyphs
	// - post Windows Vista only
	setFont( fontName );

	// 5. set file stream translation mode
	std::ios_base::sync_with_stdio( false );

	print( "Console attributes set.\n" );
	print( "Console mode set to " + std::string{getConsoleModeStr()} + '\n' );
	print( "Console ready.\n\n" );
	setDefaultColor();
	std::cout.clear();	// after this one we are ready to print
}

KeyConsole::~KeyConsole()
{
	closeConsole();
}

DWORD KeyConsole::print( const std::string &msg )
{
	m_hMode = stdout;
	m_fp = freopen( "CONOUT$", "w", m_hMode );
	m_stdDevice = STD_OUTPUT_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nWritten = 0;
	WriteConsoleA( m_hConsole, msg.c_str(), static_cast<DWORD>( msg.length() ), &nWritten, nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;
	return nWritten;
}

DWORD KeyConsole::log( const std::string &msg, LogCategory cat /*= None*/ )
{
	m_hMode = stdout;
	m_fp = freopen( "CONOUT$", "w", m_hMode );
	m_stdDevice = STD_OUTPUT_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	std::string categoryStr;
	if ( cat != LogCategory::None )
	{
		using namespace std::string_literals;
		categoryStr = std::string{magic_enum::enum_name(cat)} + ": "s;
	}
	const std::string logStr = categoryStr + msg;

	DWORD nWritten = 0;
	WriteConsoleA( m_hConsole, logStr.c_str(), static_cast<DWORD>( logStr.length() ), &nWritten, nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;
	return nWritten;
}

DWORD KeyConsole::error( const std::string &msg )
{
	m_hMode = stderr;
	m_fp = freopen( "CONERR$", "w", m_hMode );
	m_stdDevice = STD_ERROR_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nWritten = 0;
	WriteConsoleA( m_hConsole, msg.c_str(), static_cast<DWORD>( msg.length() ), &nWritten, nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;
	return nWritten;
}

std::string KeyConsole::read( const uint32_t maxChars )
{
	m_hMode = stdin;
	m_fp = freopen( "CONIN$", "w", m_hMode );
	m_stdDevice = STD_INPUT_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nRead = 0;
	std::string buff;
	buff.reserve( maxChars );
	buff.resize( maxChars );
	ReadConsoleA( m_hConsole, buff.data(), maxChars, &nRead, nullptr );
	buff.resize( nRead - 2 );	// removing superfluous size including the size needed for \r\n
	ASSERT_HRES_WIN32_IF_FAILED;
	return buff;
}

void KeyConsole::setTitle( const std::string& title ) const
{
	SetConsoleTitleW( util::s2ws( title ).c_str() );
}

bool KeyConsole::closeConsole()
{
	if ( !FreeConsole() )
	{
		MessageBoxW( nullptr, L"Failed to close the console!", L"Console Error", MB_ICONERROR );
		return false;
	}
	fclose( m_fp );
	fclose( m_hMode );
	return true;
}

const HWND KeyConsole::getWindowHandle() const noexcept
{
	const char* phTitle = "console placeholder title";
	setTitle( phTitle );

	SleepTimer::sleepFor( 10 );	// ensure window title has been updated

	const HWND consoleHWnd = FindWindowW( nullptr, util::s2ws( phTitle ).c_str() );
	ASSERT_HRES_WIN32_IF_FAILED;

	// restore original title
	setTitle( MAKE_CONSOLE_DEFAULT_TITLE );

	return consoleHWnd;
}

int KeyConsole::getConsoleMode() const noexcept
{
	if ( m_hMode == stdout )
	{
		return 0;
	}
	else if ( m_hMode == stdin )
	{
		return 1;
	}
	else if ( m_hMode == stderr )
	{
		return 2;
	}
	return -1;
}

std::string KeyConsole::getConsoleModeStr() const noexcept
{
	std::string strMode;
	switch ( m_stdDevice )
	{
	case STD_OUTPUT_HANDLE:
		strMode = "out";
		break;
	case STD_ERROR_HANDLE:
		strMode = "err";
		break;
	case STD_INPUT_HANDLE:
		strMode = "in";
		break;
	default:
		break;
	}
	return strMode;
}

const HANDLE KeyConsole::getHandle() const noexcept
{
	return m_hConsole;
}

std::string KeyConsole::getTitle() const noexcept
{
	wchar_t consoleTitle[MAX_CONSOLE_TITLE_LEN];
	GetConsoleTitleW( consoleTitle, MAX_CONSOLE_TITLE_LEN );
	ASSERT_HRES_WIN32_IF_FAILED;
	return util::ws2s( consoleTitle );
}

uint32_t KeyConsole::getConsoleCodePage() const noexcept
{
	return GetConsoleCP();
}

int32_t KeyConsole::setConsoleCodePage( const uint32_t cp )
{
	return SetConsoleCP( cp );
}

void KeyConsole::setFont( const std::string &fontName )
{
	CONSOLE_FONT_INFOEX cfie;
	const auto sz = sizeof( CONSOLE_FONT_INFOEX );
	ZeroMemory( &cfie, sz );
	cfie.cbSize = sz;
	cfie.dwFontSize.X = 0;
	cfie.dwFontSize.Y = 14;
	cfie.FontFamily = FF_DONTCARE;
	cfie.FontWeight = FW_NORMAL;
	wcscpy( cfie.FaceName, util::s2ws( fontName ).data() );
	SetCurrentConsoleFontEx( m_hConsole, false, &cfie );
	getConsoleInfo( m_hConsole );
}

int32_t KeyConsole::setCurcorPos( const _COORD xy /* = { 0,0 } */ )
{
	return SetConsoleCursorPosition( m_hConsole, xy );
}

bool KeyConsole::setColor( const WORD attributes )
{
	BOOL ret = SetConsoleTextAttribute( m_hConsole, attributes );
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
		return false;
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	ret = GetConsoleScreenBufferInfo( m_hConsole, &csbi );
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
		return false;
	}
	m_consoleAttributes = csbi.wAttributes;
	return true;
}

WORD KeyConsole::getConsoleTextAttributes() const noexcept
{
	return m_consoleAttributesDefault;
}

WORD KeyConsole::getConsoleDefaultTextAttributes() const noexcept
{
	return m_consoleAttributesDefault;
}

void KeyConsole::show() const
{
	ShowWindow( GetConsoleWindow(), SW_SHOW );
}

void KeyConsole::hide() const
{
	ShowWindow( GetConsoleWindow(), SW_HIDE );
}

DWORD KeyConsole::getFontFamily( const HANDLE h )
{
	CONSOLE_FONT_INFO cfi;
	BOOL conFont = GetCurrentConsoleFont( h, false, &cfi );
	std::cout << conFont << "\nnFont=" << cfi.nFont << "fontSize=(" << cfi.dwFontSize.X << ',' << cfi.dwFontSize.Y << ")\n";
	return conFont ? cfi.nFont : -1;
}

void KeyConsole::getConsoleInfo( const HANDLE h )
{
	using GETNUMBEROFCONSOLEFONTS = DWORD (WINAPI* )();
	using SETCONSOLEFONT = BOOL (WINAPI*)( HANDLE hConOut, DWORD nFont );
	auto GetNumberOfConsoleFonts = (GETNUMBEROFCONSOLEFONTS) GetProcAddress( LoadLibraryW( L"KERNEL32" ), "GetNumberOfConsoleFonts" );
	auto SetConsoleFont = (SETCONSOLEFONT) GetProcAddress( LoadLibraryW( L"KERNEL32" ), "SetConsoleFont" );
	auto font = getFontFamily( h );
	std::cout << "nConsoleFonts=" << GetNumberOfConsoleFonts() << "fontName=" << font << '\n';
}

bool KeyConsole::setDefaultColor()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	BOOL ret = GetConsoleScreenBufferInfo( m_hConsole, &csbi );
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
		return false;
	}
	m_consoleAttributesDefault = csbi.wAttributes;
	return true;
}