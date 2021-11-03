#if defined _DEBUG && !defined NDEBUG

#include <iostream>
#include "console.h"
#include "os_utils.h"
#include "utils.h"

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


DWORD KeyConsole::getFontFamily( HANDLE h )
{
	CONSOLE_FONT_INFO cfi;
	BOOL conFont = GetCurrentConsoleFont( h,
		false,
		&cfi );
	std::cout << conFont
		<< "\nnFont="
		<< cfi.nFont
		<< "fontSize=("
		<< cfi.dwFontSize.X
		<< ','
		<< cfi.dwFontSize.Y
		<< ")\n";
	return conFont ?
		cfi.nFont :
		-1;
}

void KeyConsole::getConsoleInfo( HANDLE h )
{
	using GETNUMBEROFCONSOLEFONTS = DWORD (WINAPI* )();
	using SETCONSOLEFONT = BOOL (WINAPI*)( HANDLE hConOut, DWORD nFont );
	auto GetNumberOfConsoleFonts =
		(GETNUMBEROFCONSOLEFONTS) GetProcAddress( LoadLibraryW( L"KERNEL32" ),
		"GetNumberOfConsoleFonts" );
	auto SetConsoleFont = (SETCONSOLEFONT) GetProcAddress( LoadLibraryW( L"KERNEL32" ),
		"SetConsoleFont" );
	auto font = getFontFamily( h );
	std::cout << "nConsoleFonts="
		<< GetNumberOfConsoleFonts()
		<< "fontName="
		<< font
		<< '\n';
}


KeyConsole::KeyConsole( const std::string& fontName )
	:
	m_fp{nullptr},
	m_title{std::string{defaultConsoleTitle} + std::string{" "} + std::string{currentVersion}},
	m_stdDevice{STD_OUTPUT_HANDLE},
	m_hMode{stdout},
	m_hConsole{GetStdHandle( m_stdDevice )}
{
	fflush( stdout );

	// 1. Allocate console
	if ( !AllocConsole() )
	{
		MessageBoxW( nullptr,
			util::s2ws( "Unable to create Debug Console." ).data(),
			util::s2ws( "Notice" ).data(),
			MB_ICONEXCLAMATION );
	}
	
	// 2. set console title
	SetConsoleTitleW( util::s2ws( m_title ).data() );

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
	print( "Console mode set to " + getConsoleModeStr() + '\n' );
	print( "Console ready.\n\n" );
	setDefaultColor();
	std::cout.clear();	// after this one we are ready to print
}

KeyConsole::~KeyConsole()
{
	closeConsole();
}

DWORD KeyConsole::print( const std::string& msg )
{
	m_hMode = stdout;
	m_fp = freopen( "CONOUT$",
		"w",
		m_hMode );
	m_stdDevice = STD_OUTPUT_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nWritten = 0;
	auto ret = WriteConsoleA( m_hConsole,
		msg.c_str(),
		static_cast<DWORD>( msg.length() ),
		&nWritten,
		nullptr );
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}
	return nWritten;
}

DWORD KeyConsole::log( const std::string& msg )
{
	m_hMode = stderr;
	m_fp = freopen( "CONERR$",
			"w",
			m_hMode );
	m_stdDevice = STD_ERROR_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nWritten = 0;
	auto ret = WriteConsoleA( m_hConsole,
		msg.c_str(),
		static_cast<DWORD>( msg.length() ),
		&nWritten,
		nullptr );
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}
	return nWritten;
}

std::string KeyConsole::read( const uint32_t maxChars )
{
	m_hMode = stdin;
	m_fp = freopen( "CONIN$",
		"w",
		m_hMode );
	m_stdDevice = STD_INPUT_HANDLE;
	m_hConsole = GetStdHandle( m_stdDevice );

	DWORD nRead = 0;
	std::string buff;
	buff.reserve( maxChars );
	buff.resize( maxChars );
	auto ret = ReadConsoleA( m_hConsole,
		buff.data(),
		maxChars,
		&nRead,
		nullptr );
	buff.resize( nRead - 2 );	// removing superfluous size including the size needed for \r\n
	if ( !ret )
	{
		OutputDebugStringW( util::printHresultErrorDescriptionW( HRESULT_FROM_WIN32( GetLastError() ) ).data() );
	}
	return buff;
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

uint32_t KeyConsole::getConsoleCodePage() const noexcept
{
	return GetConsoleCP();
}

HANDLE KeyConsole::getHandle() const noexcept
{
	return m_hConsole;
}

int32_t KeyConsole::setConsoleCodePage( uint32_t cp )
{
	return SetConsoleCP( cp );
}

void KeyConsole::setFont( const std::string& fontName )
{
	CONSOLE_FONT_INFOEX cfie;
	const auto sz = sizeof( CONSOLE_FONT_INFOEX );
	ZeroMemory( &cfie, sz );
	cfie.cbSize = sz;
	cfie.dwFontSize.X = 0;
	cfie.dwFontSize.Y = 14;
	cfie.FontFamily = FF_DONTCARE;
	cfie.FontWeight = FW_NORMAL;
	wcscpy( cfie.FaceName,
		util::s2ws( fontName ).data() );
	SetCurrentConsoleFontEx( m_hConsole,
		false,
		&cfie );
	getConsoleInfo( m_hConsole );
}

int32_t KeyConsole::setCurcorPos( _COORD xy /* = { 0,0 } */ )
{
	return SetConsoleCursorPosition( m_hConsole,
		xy );
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

bool KeyConsole::setColor( WORD attributes )
{
	BOOL ret = SetConsoleTextAttribute( m_hConsole,
		attributes );
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
	ShowWindow( GetConsoleWindow(),
		SW_SHOW );
}

void KeyConsole::hide() const
{
	ShowWindow( GetConsoleWindow(),
		SW_HIDE );
}

bool KeyConsole::closeConsole()
{
	if ( !FreeConsole() )
	{
		MessageBoxW( nullptr,
			L"Failed to close the console!",
			L"Console Error",
			MB_ICONERROR );
		return false;
	}
	fclose( m_fp );
	fclose( m_hMode );
	return true;
}

KeyConsole& KeyConsole::getInstance() noexcept 
{
	if ( m_pInstance == nullptr )
	{
		m_pInstance = new KeyConsole;
	}
	return *m_pInstance;
}

void KeyConsole::resetInstance()
{
	if ( m_pInstance != nullptr )
	{
		delete m_pInstance;
	}
}


#endif