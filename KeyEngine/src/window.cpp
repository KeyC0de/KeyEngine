#include <sstream>
#include <string>
#include <windowsx.h>
#include "window.h"
#include "graphics.h"
#ifndef FINAL_RELEASE
#	include "imgui_impl_win32.h"
#endif
#include "utils.h"
#include "os_utils.h"
#include "math_utils.h"
#include "console.h"
#include "assertions_console.h"
#include "graphics_mode.h"
#include "windows_message_map.h"
#include "../resource.h"
#include "winuser.h"
#include "settings_manager.h"

#define IDT_TIMER_SPLASH_WINDOW_DESTRUCTION	1000

#if defined _DEBUG && !defined NDEBUG
#	define PRINT_WIN_MESSAGE	{\
									console.print( "win msgId: " + WindowsMessageMap::getInstance().toString( uMsg ) + "\n");\
								}
#else
#	define PRINT_WIN_MESSAGE	(void)0;
#endif


Window::WindowClass::WindowClass( const char *name,
	const WNDPROC windowProcedure,
	const ColorBGRA bgColor /*= {255, 255, 255}*/ )
	:
	m_name{name}
{
	ASSERT( name != nullptr, "Class name is null!" );

	HINSTANCE hInstance = THIS_INSTANCE;
	ASSERT( hInstance != nullptr, "HINSTANCE is null!" );

	UINT classStyles = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		//| CS_DBLCLKS
		// CS_VREDRAW allows sending WM_SIZE message when either the height or the width of the client area are changed
	if ( name != MAIN_WINDOW_CLASS_NAME )
	{
		classStyles = 0u;
	}

	WNDCLASSEX wc{};
	wc.cbSize = sizeof WNDCLASSEX;
	wc.style = classStyles;
	wc.lpfnWndProc = windowProcedure;
	wc.hInstance = hInstance;
	wc.hbrBackground = CreateSolidBrush( static_cast<COLORREF>( bgColor ) );
	std::wstring wname = util::s2ws( name );
	wc.lpszClassName = wname.data();
	//wc.cbClsExtra = 0;
	//wc.cbWndExtra = 0;	// extra bytes
	//wc.lpszMenuName = nullptr;

	wc.hCursor = LoadCursorW( hInstance, MAKEINTRESOURCEW( IDC_HOMM_INSPIRED ) );
	ASSERT_HRES_WIN32_IF_FAILED;
	wc.hIcon = static_cast<HICON>( LoadImageW( hInstance, MAKEINTRESOURCEW( IDI_ICON_APP ), IMAGE_ICON, 64, 43, 0u ) );
	ASSERT_HRES_WIN32_IF_FAILED;
	wc.hIconSm = static_cast<HICON>( LoadImageW( hInstance, MAKEINTRESOURCEW( IDI_ICON_APP_SMALL ), IMAGE_ICON, 32, 22, 0u ) );
	ASSERT_HRES_WIN32_IF_FAILED;

	if ( name == SPLASH_WINDOW_CLASS_NAME )
	{
		wc.hCursor = nullptr;
		wc.hIcon = nullptr;
		wc.hIconSm = nullptr;
	}

	m_classAtom = RegisterClassExW( &wc );
	ASSERT_HRES_WIN32_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	using namespace std::string_literals;
	console.log( "\n\nApplication Instance: "s
		+ util::toHexString( hInstance )
		+ "\n\nWindow Class: "s
		+ m_name
		+ " registered: "s
		+ std::to_string( m_classAtom )
		+ " (class atom)\n\n"s );
#endif
}

Window::WindowClass::~WindowClass() noexcept
{
	UnregisterClassW( util::s2ws( m_name ).data(), GetModuleHandleW( nullptr ) );
}

Window::WindowClass::WindowClass( WindowClass &&rhs ) noexcept
	:
	m_name{std::move( rhs.m_name )},
	m_classAtom{rhs.m_classAtom}
{

}

Window::WindowClass& Window::WindowClass::operator=( WindowClass &&rhs ) noexcept
{
	WindowClass tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

const std::string& Window::WindowClass::getName() noexcept
{
	return m_name;
}

Window::Window( const int width,
	const int height,
	const char *name,
	const char *className,
	const WNDPROC windowProcedure,
	const int x,
	const int y,
	const ColorBGRA bgColor /*= {255, 255, 255}*/,
	const HMENU hMenu /*= nullptr*/,
	const Window *parent /*= nullptr*/ )
	:
	m_windowClass{className, windowProcedure, bgColor},
	m_name{name},
	m_clipboardFormats{0}
{
	uint32_t windowExStyles = WS_EX_OVERLAPPEDWINDOW;
	uint32_t windowStyles = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;	// disable both maximizing and resizing
	if ( className == MODAL_DIALOG_CLASS_NAME )
	{
		windowExStyles = WS_EX_DLGMODALFRAME | WS_EX_TOPMOST;
		windowStyles = WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_CHILD;
	}
	else if ( className == SPLASH_WINDOW_CLASS_NAME )
	{
		windowExStyles = WS_EX_LAYERED
			| WS_EX_TOOLWINDOW;	// don't appear in the taskbar or ALT+TAB list
		windowStyles = WS_VISIBLE | WS_POPUP;
	}

	RECT rect{0, 0, width, height};
	BOOL ret = AdjustWindowRectEx( &rect, windowStyles, ( hMenu == nullptr ) ? FALSE : TRUE, windowExStyles );
	if ( !ret )
	{
		THROW_WINDOW_EXCEPTION( "Failed to adjust window rectangle" );
	}

	const int adjustedWindowWidth = rect.right - rect.left;
	const int adjustedWindowHeight = rect.bottom - rect.top;

	const std::wstring classNameWstr = util::s2ws( className );
	const std::wstring nameWstr = util::s2ws( name );

	if ( className == MODAL_DIALOG_CLASS_NAME )
	{
		m_hWnd = (HWND) ::DialogBoxW( THIS_INSTANCE, MAKEINTRESOURCEW( IDD_HELP_DIALOG ), parent->getHandle(), windowProcedure );
	}
	else if ( className == MODELESS_DIALOG_CLASS_NAME )
	{
		// #TODO: https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes#creating-a-modeless-dialog-box
	}
	else if ( className == SPLASH_WINDOW_CLASS_NAME )
	{
		m_hWnd = CreateWindowExW( windowExStyles,
			classNameWstr.c_str(),
			nameWstr.c_str(),
			windowStyles,
			x,
			y,
			adjustedWindowWidth,
			adjustedWindowHeight,
			parent == nullptr ? HWND_DESKTOP : parent->getHandle(),
			nullptr,
			THIS_INSTANCE,
			nullptr );
		// The window now exists, but isn't sized or positioned correctly, and has no content. UpdateLayeredWindow() can be used to correct all these problems at once.
	}
	else
	{
		m_hWnd = CreateWindowExW( windowExStyles,
			classNameWstr.c_str(),
			nameWstr.c_str(),
			windowStyles,
			x,
			y,
			adjustedWindowWidth,
			adjustedWindowHeight,
			parent == nullptr ? HWND_DESKTOP : parent->getHandle(),
			nullptr,	// menu - if modeless window you can use (HMENU)IDC_MODELESS
			THIS_INSTANCE,
			this );
	}
	ASSERT_HRES_WIN32_IF_FAILED;
	if ( !m_hWnd )
	{
		THROW_WINDOW_EXCEPTION( "Failed to create Window" );
	}

	// set title
	static const short maxTitleCharSize = 128i16;
	wchar_t title[maxTitleCharSize];
	GetWindowTextW( m_hWnd, title, maxTitleCharSize );
	m_title = util::ws2s( title );

	if ( className != MAIN_WINDOW_CLASS_NAME )
	{
		// special Window creation has been completed
		return;
	}

	m_info.cbSize = sizeof WINDOWINFO;
	GetWindowInfo( m_hWnd,
		&m_info );
	ASSERT_HRES_WIN32_IF_FAILED;

	// mouse raw input
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01u;	// mouse page
	rid.usUsage = 0x02u;		// mouse usage
	rid.dwFlags = 0u;
	rid.hwndTarget = nullptr;
	ret = RegisterRawInputDevices( &rid, 1u, sizeof RAWINPUTDEVICE );
	ASSERT_HRES_WIN32_IF_FAILED;

	// (keyboard) accelerators are keystrokes or a combination of keystrokes that generate a WM_COMMAND or WM_SYSCOMMAND message for an application
	if ( s_hAcceleratorTable == nullptr )
	{
		s_hAcceleratorTable = LoadAcceleratorsW( THIS_INSTANCE, MAKEINTRESOURCEW( ID_ACCEL_TABLE_APP ) );
		ASSERT_HRES_WIN32_IF_FAILED;
	}

	// register other clipboard formats to handle:
	// we assume that atchTemp can contain the format name and a null-terminator, otherwise it is truncated
	wchar_t unicodeClipboardFormat[32];
	LoadStringW( THIS_INSTANCE, IDS_CLIPBOARD_UNICODE_FORMAT, unicodeClipboardFormat, sizeof( unicodeClipboardFormat) / sizeof( WCHAR ) );
	m_clipboardFormats[0] = RegisterClipboardFormatW( unicodeClipboardFormat );
	ASSERT_HRES_WIN32_IF_FAILED;

	if ( hMenu )
	{
		showMenu( hMenu );
	}

	setupTrayIcon();

	if constexpr( gph_mode::get() == gph_mode::_3D )
	{
#ifndef FINAL_RELEASE
		// initialize ImGui
		ImGui_ImplWin32_Init( m_hWnd );
#endif
	}

	// setup Graphics for the Window
	m_pGraphics = std::make_unique<Graphics>( m_hWnd, adjustedWindowWidth, adjustedWindowHeight );

	// show window
	ret = UpdateWindow( m_hWnd );
	if ( ret == FALSE )
	{
		THROW_WINDOW_EXCEPTION( "Failed to update the client area of the window." );
	}
	ret = ShowWindow( m_hWnd, SW_SHOWDEFAULT );
}

Window::Window( Window &&rhs ) noexcept
	:
	m_windowClass{std::move( rhs.m_windowClass )},
	m_bCursorEnabled{std::move( rhs.m_bCursorEnabled )},
	m_name{std::move( rhs.m_name )},
	m_hWnd{std::move( rhs.m_hWnd )},
	m_pGraphics{std::move( rhs.m_pGraphics )},
	m_info{std::move( rhs.m_info )},
	m_clipboardFormats{std::move( rhs.m_clipboardFormats )},
	m_hTrayIconPopupMenu{rhs.m_hTrayIconPopupMenu},
	m_trayIconData{std::move( m_trayIconData )}
{
	s_keyboard = std::move( rhs.s_keyboard );
	rhs.s_keyboard = {};
	s_mouse = std::move( rhs.s_mouse );
	rhs.s_mouse = {};
	s_rawInputBuffer = std::move( rhs.s_rawInputBuffer );
	rhs.s_rawInputBuffer = {};
}

Window& Window::operator=( Window &&rhs ) noexcept
{
	std::swap( *this, rhs );
	return *this;
}

Window::~Window()
{
	const auto& className = m_windowClass.getName();
	if ( className == MAIN_WINDOW_CLASS_NAME )
	{
		deleteTrayIcon();
		// a menu assigned to a window is automatically destroyed with the window
		//DestroyMenu( m_hTrayIconPopupMenu );	// #FIXME: if the tray icon is hidden in the collapsed section this function will fail and trigger an exception
		//ASSERT_HRES_WIN32_IF_FAILED;
	}

	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		if ( className == MAIN_WINDOW_CLASS_NAME )
		{
#ifndef FINAL_RELEASE
			ImGui_ImplWin32_Shutdown();
#endif
		}
	}

	// HWND should be already destroyed by this point, as PostQuitMessage is part of window's destruction
	// so getDc should return 0 the DC for the entire screen
	const HDC hDc = getDc();
	if ( hDc )
	{
		const BOOL bReleased = ReleaseDC( m_hWnd, hDc );
		ASSERT( bReleased == 1, "The window's device context was not properly released for use by other applications!" );

		DestroyWindow( m_hWnd );
		ASSERT_HRES_WIN32_IF_FAILED;
	}
}

void Window::setEnable( const bool b )
{
	EnableWindow( m_hWnd, b ? TRUE : FALSE );
}

void Window::setOnTop()
{
	SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
	// redraw to prevent the window blank
	RedrawWindow( m_hWnd, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN );
}

const HWND Window::setFocus()
{
	HWND previouslyFocusedWindow = SetFocus( m_hWnd );
	return previouslyFocusedWindow;
}

bool Window::hasParent() const noexcept
{
	if ( !getParent() )
	{
		return false;
	}
	return ( getParent() == GetDesktopWindow() ) ?
		false :
		true;
}

const HWND Window::getParent() const noexcept
{
	return GetParent( m_hWnd );
}

void Window::setTitle( const std::string &title )
{
	SetWindowTextW( m_hWnd, util::s2ws( title ).data() );
	ASSERT_HRES_WIN32_IF_FAILED;
}

const std::string& Window::getTitle() const noexcept
{
	return m_title;
}

void Window::enableCursor() noexcept
{
	m_bCursorEnabled = true;
	showCursor();
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		enableImGuiMouse();
	}
	freeCursor();
}

void Window::disableCursor() noexcept
{
	m_bCursorEnabled = false;
	hideCursor();
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		disableImGuiMouse();
	}
	confineCursor();
}

bool Window::isCursorEnabled() const noexcept
{
	return m_bCursorEnabled;
}

HDC Window::getDc() const noexcept
{
	return GetWindowDC( m_hWnd );
}

bool Window::displayMessageBoxYesNo( const std::string &title,
	const std::string &message ) const
{
	return MessageBoxW( m_hWnd, util::s2ws( message ).c_str(), util::s2ws( title ).c_str(), MB_OKCANCEL );
}

bool Window::isActive() const noexcept
{
	return GetActiveWindow() == m_hWnd;
}

const std::string& Window::getName() const noexcept
{
	return m_name;
}

void Window::minimize()
{
	int ret = ShowWindow( m_hWnd, SW_HIDE );
	ASSERT( ret != 0, "The window is already hidden!" );
}

void Window::restore()
{
	int ret = ShowWindow( m_hWnd, SW_SHOW );
	ASSERT( ret != 0, "The window is already visible!" );
	SetForegroundWindow( m_hWnd );
}

void Window::setupTrayIcon()
{
	ZeroMemory( &m_trayIconData, sizeof NOTIFYICONDATA );
	m_trayIconData.cbSize = sizeof NOTIFYICONDATA;
	m_trayIconData.hWnd = m_hWnd;
	m_trayIconData.uID = IDI_ICON_TRAY;	// stored in the wParam data of the uMsg
	m_trayIconData.uFlags = NIF_ICON |	// load icon
		NIF_TIP	|						// support icon tooltip
		NIF_INFO |						// display a Windows balloon notification
		NIF_MESSAGE;					// support callback message
	m_trayIconData.hIcon = LoadIconW( GetModuleHandleW( nullptr ), MAKEINTRESOURCEW( IDI_ICON_TRAY ) );
	m_trayIconData.uCallbackMessage = WM_TRAY_ICON;
	const std::wstring trayIconTooltipText{L"KeyEngine is relaxing in the System Tray."};
	wcsncpy( m_trayIconData.szTip, trayIconTooltipText.c_str(), trayIconTooltipText.size() );

	const std::wstring trayIconWindowsBalloonNotificationTitle{L"KeyEngine Windows balloon notification"};
	wcsncpy( m_trayIconData.szInfoTitle, trayIconWindowsBalloonNotificationTitle.c_str(), trayIconWindowsBalloonNotificationTitle.size() );

	const std::wstring trayIconWindowsBalloonNotificationDesc{L"The powerful KeyEngine is energized!"};
	wcsncpy( m_trayIconData.szInfo, trayIconWindowsBalloonNotificationDesc.c_str(), trayIconWindowsBalloonNotificationDesc.size() );

	//m_trayIconData.dwInfoFlags = //misc extra flags
}

void Window::showTrayIcon() noexcept
{
	BOOL ret = Shell_NotifyIconW( NIM_ADD, &m_trayIconData );
	//ASSERT( ret == TRUE, "Tray icon setup unsuccessful!" );
}

void Window::deleteTrayIcon() noexcept
{
	BOOL ret = Shell_NotifyIconW( NIM_DELETE, &m_trayIconData );
	//ASSERT( ret == TRUE, "Tray icon setup unsuccessful!" );
	Sleep( 10 );
}

bool Window::isHidden() const noexcept
{
	return !IsIconic( m_hWnd );
}

bool Window::isMinimized() const noexcept
{
	return !IsIconic( m_hWnd );
}

bool Window::isMaximized() const noexcept
{
	return !IsZoomed( m_hWnd );
}

Window::operator bool() const noexcept
{
	return m_hWnd != nullptr;
}

std::optional<int> Window::messageLoop() noexcept
{
	MSG msg;
	while ( PeekMessageW( &msg, nullptr, 0u, 0u, PM_REMOVE ) )
	{
		const unsigned uMsg = msg.message;
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
		PRINT_WIN_MESSAGE( uMsg );
#endif

		// check for quit because peekmessage does not signal this via return val
		if ( uMsg == WM_QUIT || uMsg == WM_CLOSE || uMsg == WM_DESTROY )
		{
			if ( s_timerEvent )
			{
				KillTimer( m_hWnd, s_timerEvent );
			}
			return (int)msg.wParam;
		}
		if ( !TranslateAcceleratorW( m_hWnd, s_hAcceleratorTable, &msg ) )
		{
			TranslateMessage( &msg );
			DispatchMessageW( &msg );
		}
	}
	return std::nullopt;
}

Graphics& Window::getGraphics()
{
	return *m_pGraphics;
}

const HWND Window::getHandle() const noexcept
{
	return m_hWnd;
}

WINDOWINFO Window::getInfo() const noexcept
{
	return m_info;
}

Keyboard& Window::getKeyboard() noexcept
{
	return s_keyboard;
}

Mouse& Window::getMouse() noexcept
{
	return s_mouse;
}

bool Window::isDescendantOf( const HWND targethWnd,
	const HWND parent ) noexcept
{
	return IsChild( parent, targethWnd ) == 0 ?
		false :
		true;
}

HBITMAP Window::convertHiconToHbitmap( HICON hIcon )
{
	ICONINFO iconinfo;
	GetIconInfo( hIcon, &iconinfo );
	return iconinfo.hbmColor;
}

HICON Window::convertHbitmapToHicon( HBITMAP hBitmap )
{
	BITMAP bitmap{};
	int ret = GetObjectW( hBitmap, sizeof bitmap, &bitmap );
	ASSERT( ret != 0, "Couldn't GetObject" );

	HBITMAP hbmMask = ::CreateCompatibleBitmap( ::GetDC( nullptr ), bitmap.bmWidth, bitmap.bmHeight );

	ICONINFO iconInfo{0};
	iconInfo.fIcon = TRUE;
	iconInfo.hbmColor = hBitmap;
	iconInfo.hbmMask = hbmMask;

	HICON hIcon = ::CreateIconIndirect( &iconInfo );

	::DeleteObject( hbmMask );

	return hIcon;
}

void Window::saveClipboardTextAsVar()
{
	HANDLE hClipboard;
	char *clipboardText;

	if ( OpenClipboard( nullptr ) )
	{
		hClipboard = GetClipboardData( CLIPBOARD_TEXT_FORMAT );
		clipboardText = (char*)hClipboard;
#if defined _DEBUG && !defined NDEBUG
		KeyConsole& console = KeyConsole::getInstance();
		console.print( "Clipboard copied text: " + std::string{clipboardText} + "\n" );
#endif
		CloseClipboard();
	}
}

void Window::setupSplashWindow( HBITMAP hSplashBitmap )
{
	int ret;

	BITMAP bitmap{};
	ret = GetObjectW( hSplashBitmap, sizeof bitmap, &bitmap );
	ASSERT( ret != 0, "Could not retrieve information out of the specified Splash image bitmap.");

	SIZE splashSize = {bitmap.bmWidth, bitmap.bmHeight};

	// center splash window on parent window
	RECT rect;
	GetClientRect( m_hWnd, &rect );
	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;

	HWND hWndParent = nullptr;
	if ( !hasParent() )
	{
		return;
	}
	hWndParent = getParent();

	RECT parentRect;
	GetWindowRect( hWndParent, &parentRect );

	const int parentWidth = parentRect.right - parentRect.left;
	const int parentHeight = parentRect.bottom - parentRect.top;

	const int x = parentRect.left + ( parentWidth / 2 ) - ( width / 2 );
	const int y = parentRect.top + ( parentHeight / 2 ) - ( height / 2 );

	POINT ptOriginSrc{0,0};
	POINT ptOriginDest{x,y};

	HDC hDcScreenDest = GetDC( nullptr );
	// create "memory dc" or "compatible dc" - a dc special for bitmaps - to hold the splash bitmap
	HDC hDcMemSrc = CreateCompatibleDC( hDcScreenDest );

	// select the bitmap into the DC
	HBITMAP hBmpTemp = (HBITMAP) SelectObject( hDcMemSrc, hSplashBitmap );
	if ( hBmpTemp == nullptr || hBmpTemp == HGDI_ERROR )
	{
		THROW_WINDOW_EXCEPTION( "Couldn't create Splash Image" );
	}

	// use the source image's Alpha for blending
	BLENDFUNCTION blendFun{0};
	blendFun.BlendOp = AC_SRC_OVER;
	blendFun.SourceConstantAlpha = 255ui8;
	blendFun.AlphaFormat = AC_SRC_ALPHA;
	blendFun.BlendFlags = 0i8;

	/*
	#pragma comment( lib, "msimg32.lib" )	// required for AlphaBlend
	const int destWidth = GetSystemMetrics( SM_CXSCREEN );
	const int destHeight = GetSystemMetrics( SM_CYSCREEN );
	ret = AlphaBlend( hDcScreenDest, ptOriginDest.x, ptOriginDest.y, destWidth, destHeight, hDcMemSrc, ptOriginSrc.x, ptOriginSrc.y, width, height, blendFun );
	ASSERT( ret == TRUE, "AlphaBlend() failed!" );
	*/

	// paint the window (in the right location) with the alpha-blended bitmap
	UpdateLayeredWindow( m_hWnd, hDcScreenDest, &ptOriginDest, &splashSize, hDcMemSrc, &ptOriginSrc, RGB(0, 0, 0), &blendFun, ULW_ALPHA );
	ASSERT_HRES_WIN32_IF_FAILED;

	// clear temporary objects
	SelectObject( hDcMemSrc, hBmpTemp );
	DeleteDC( hDcMemSrc );
	ReleaseDC( nullptr, hDcScreenDest );

	// show window
	ret = UpdateWindow( m_hWnd );
	if ( ret == FALSE )
	{
		THROW_WINDOW_EXCEPTION( "Failed to update the client area of the window." );
	}
	ret = ShowWindow( m_hWnd, SW_SHOWDEFAULT );
}

void Window::confineCursor() noexcept
{
	RECT rect;
	GetClientRect( m_hWnd, &rect );
	MapWindowPoints( m_hWnd, nullptr, reinterpret_cast<POINT*>( &rect ), 2u );
	ClipCursor( &rect );
}

void Window::freeCursor() noexcept
{
	ClipCursor( nullptr );
}

void Window::hideCursor() noexcept
{
	//ShowCursor( FALSE );
	while ( ShowCursor( FALSE ) >= 0 );
}

void Window::showCursor() noexcept
{
	//ShowCursor( TRUE );
	while ( ShowCursor( TRUE ) < 0 );
}

void Window::enableImGuiMouse() noexcept
{
#ifndef FINAL_RELEASE
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
#endif
}

void Window::disableImGuiMouse() noexcept
{
#ifndef FINAL_RELEASE
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
#endif
}

int Window::messageBoxPrintf( const HWND window,
	const TCHAR *caption,
	const TCHAR *format,
	... )
{
	TCHAR szBuffer[1024];
	va_list pArgs;
	va_start( pArgs, format );
	vswprintf_s( szBuffer, sizeof szBuffer / sizeof TCHAR, format, pArgs );
	va_end( pArgs );
	return MessageBoxW( window, szBuffer, caption, 0u );
}

LRESULT CALLBACK Window::windowProc( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	console.log( "windowProc() entered\n" );
	PRINT_WIN_MESSAGE;
#endif

	if ( uMsg == WM_NCCREATE )
	{
#if defined _DEBUG && !defined NDEBUG
		console.log( "windowProc() WM_NCCREATE message\n" );
#endif
		const CREATESTRUCTW *const cs = (CREATESTRUCTW*) lParam;
		Window *this_wnd = reinterpret_cast<Window*>( cs->lpCreateParams );

		this_wnd->m_hWnd = hWnd;

		SetWindowLongPtrW( hWnd, GWLP_USERDATA, (LONG_PTR) this_wnd );
		SetWindowLongPtrW( hWnd, GWLP_WNDPROC, (LONG_PTR) &Window::windowProcDelegate );
		return this_wnd->windowProcDelegate( hWnd, uMsg, wParam, lParam );
	}

	// if we get a msg before the WM_NCCREATE msg handle it with the default windowProc
	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK Window::windowProcDelegate( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

	Window *this_wnd = reinterpret_cast<Window*>( GetWindowLongPtrW( hWnd, GWLP_USERDATA ) );
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		return this_wnd->windowProc_impl3d( hWnd, uMsg, wParam, lParam );
	}
	else
	{
		return this_wnd->windowProc_impl2d( hWnd, uMsg, wParam, lParam );
	}
}

LRESULT Window::windowProc_impl2d( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

	switch( uMsg )
	{
	case WM_CREATE:
	{
		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	case WM_KILLFOCUS:
	{
		// clear keyboard button states when window loses focus to prevent keyboard input getting "stuck"
		s_keyboard.clearKeyStates();
		break;
	}
	case WM_ACTIVATEAPP:	// Sent to the top level window belonging to a different application than the active window is about to be activated as well as the top level window of this application
	case WM_ACTIVATE:	// message sent to the window being activated and to the window being deactivated
	{
#if defined _DEBUG && !defined NDEBUG
		using namespace std::string_literals;
		console.log( "'WM_ACTIVATEAPP' : message ID received\n"s );
#endif
		if ( !m_bCursorEnabled )
		{
			if ( wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE )
			{
				confineCursor();
				hideCursor();
			}
			else // WA_INACTIVE
			{
				freeCursor();
				showCursor();
			}
		}

		showTrayIcon();
		break;
	}
	/// Keyboard Messages
	case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		[[fallthrough]];
	case WM_SYSKEYDOWN:
	{
		if ( (lParam | 0x40000000) && !s_keyboard.isAutorepeatEnabled() ) // filter autorepeat
		{
			s_keyboard.onKeyPressed( static_cast<unsigned char>( wParam ) );
		}
		break;
	}
	case WM_KEYUP:
		[[fallthrough]];
	case WM_SYSKEYUP:
	{
		s_keyboard.onKeyReleased( static_cast<unsigned char>( wParam ) );
		break;
	}
	case WM_CHAR:
	{
		s_keyboard.onChar( static_cast<unsigned char>( wParam ) );
		break;
	}
	/// Mouse Messages
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		// cursorless exclusive gets first dibs
		if ( !m_bCursorEnabled )
		{
			if ( !s_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				s_mouse.onMouseEnterWindow();
				hideCursor();
			}
			break;
		}
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if ( pt.x >= 0 && pt.x < getWidth() && pt.y >= 0 && pt.y < getHeight() )
		{
			s_mouse.onMouseMove( pt.x, pt.y );
			if ( !s_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				s_mouse.onMouseEnterWindow();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			//if ( wParam & (MK_LBUTTON | MK_RBUTTON) )
			// if we're dragging while the cursor hot pixel is outside the windows bounds
			// keep getting mouse events even when it's outside the window
			if ( s_mouse.isLmbPressed() || s_mouse.isRmbPressed() )
			{
				s_mouse.onMouseMove( pt.x, pt.y );
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				s_mouse.onMouseLeaveWindow();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		SetForegroundWindow( hWnd );
		if ( !m_bCursorEnabled )
		{
			confineCursor();
			hideCursor();
		}
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onLmbPressed( pt.x, pt.y );
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onRmbPressed( pt.x, pt.y );
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onLmbReleased( pt.x, pt.y );
		// release mouse if outside of a window
		if ( pt.x < 0 || pt.x >= getWidth() || pt.y < 0 || pt.y >= getHeight() )
		{
			ReleaseCapture();
			s_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onRmbReleased( pt.x, pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= getWidth() || pt.y < 0 || pt.y >= getHeight() )
		{
			ReleaseCapture();
			s_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		s_mouse.onWheelDelta( pt.x, pt.y, delta );
		break;
	}
	/// Raw Input Messages
	case WM_INPUT:
	{
		if ( !s_mouse.isRawInputEnabled() )
		{
			break;
		}
		unsigned size = -1;
		// first get the size of the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, nullptr, &size, sizeof( RAWINPUTHEADER ) ) == -1 )
		{
			// bail msg processing if error
			break;
		}
		s_rawInputBuffer.resize( size );
		// read in the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, s_rawInputBuffer.data(), &size, sizeof( RAWINPUTHEADER ) ) != size )
		{
			// bail msg processing if error
			break;
		}
		// process the raw input data
		auto &ri = reinterpret_cast<const RAWINPUT&>( *s_rawInputBuffer.data() );
		if ( ri.header.dwType == RIM_TYPEMOUSE && ( ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0 ) )
		{
			s_mouse.onRawDelta( ri.data.mouse.lLastX, ri.data.mouse.lLastY );
		}
		break;
	}
	}//switch

	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

LRESULT Window::windowProc_impl3d( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

#ifndef FINAL_RELEASE
	// ImGui windowProc messages
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) )
	{
		return true;
	}
	const auto &imGuiIoContext = ImGui::GetIO();
#endif

	static bool s_bMinimized = false;
	static bool s_bCurrentlyResizing = false;

	// custom messages (create with RegisterWindowMessage)

	// Predefined Windows messages
	switch( uMsg )
	{
	case WM_CREATE:	// send to a window being initialized. Controls - child windows, set default values for controls
	{
#if defined _DEBUG && !defined NDEBUG
		using namespace std::string_literals;
		console.log( "'WM_CREATE' : window created\n"s );
#endif

		// create tray icon
		m_hTrayIconPopupMenu = CreatePopupMenu();
		int ret = AppendMenuW( m_hTrayIconPopupMenu, MF_STRING, IDM_TRAY_ICON_EXIT, L"Exit the Application." );
		//ASSERT_HRES_WIN32_IF_FAILED;	// GetLastError crashes here - unknown Windows Bug

		// create splash Window
		const int splashWindowWidth = 480;
		const int splashWindowHeight = 643;
		const int screenWidth = GetSystemMetrics( SM_CXSCREEN );
		const int screenHeight = GetSystemMetrics( SM_CYSCREEN );
		const int x = screenWidth / 2 - splashWindowWidth / 2;
		const int y = screenHeight / 2 - splashWindowHeight / 2;

		m_pSplash = std::make_unique<Window>( splashWindowWidth, splashWindowHeight, "KeyEngine Splash Window", SPLASH_WINDOW_CLASS_NAME, Window::splashWindowProc, x, y, ColorBGRA{0, 0, 0}, nullptr, this );

		HBITMAP hSplashBitmap = LoadBitmapW( THIS_INSTANCE, MAKEINTRESOURCEW( IDB_SPLASH_WINDOW ) );
		ASSERT_HRES_WIN32_IF_FAILED;
		m_pSplash->setupSplashWindow( hSplashBitmap );

		const DWORD msAfterWhichToCloseSplashWindow = 4000u;
		s_timerEvent = SetTimer( m_pSplash->getHandle(), IDT_TIMER_SPLASH_WINDOW_DESTRUCTION, msAfterWhichToCloseSplashWindow, (TIMERPROC) splashWindowTimerProc );

		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	case WM_KILLFOCUS:
	{
		// clear keyboard button states when window loses focus to prevent keyboard input getting "stuck"
		s_keyboard.clearKeyStates();
		break;
	}
	case WM_ACTIVATEAPP:	// Sent to the top level window belonging to a different application than the active window is about to be activated as well as the top level window of this application
	case WM_ACTIVATE:	// message sent to the window being activated and to the window being deactivated
	{
#if defined _DEBUG && !defined NDEBUG
		using namespace std::string_literals;
		console.log( "'WM_ACTIVATEAPP' : message ID received\n"s );
#endif
		if ( !m_bCursorEnabled )
		{
			if ( wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE )
			{
				confineCursor();
				hideCursor();
			}
			else // WA_INACTIVE
			{
				freeCursor();
				showCursor();
			}
		}

		showTrayIcon();
		break;
	}
#if defined _DEBUG && !defined NDEBUG
	case WM_SHOWWINDOW:	// sent when the window is about to be hidden or shown
	{
		if ( wParam == TRUE )
		{
			console.log( "Window " + getTitle() + " is visible.\n" );
		}
		else
		{
			console.log( "Window " + getTitle() + " is hidden.\n" );
		}
		break;
	}
#endif
	/// Resizing messages
	case WM_SIZE:
	{
		if ( wParam == SIZE_MINIMIZED )
		{
			// the window was minimized
			if ( !s_bMinimized )
			{
				s_bMinimized = true;
				// #TODO: suspend the application at this point so that we don't waste resources
			}
		}
		else if ( s_bMinimized )
		{
			// the window was minimized and is now restored (resume from suspend)
			s_bMinimized = false;
		}
		else if ( !s_bCurrentlyResizing )
		{
			// handle the swapchain resize for maximize or unmaximize
			if ( !SettingsManager::getInstance().getSettings().bAllowWindowResize )
			{
				// another option would be to prevent window resizing and innstead resize by ingame option - choosing resolution
				break;
			}

			if ( m_pGraphics )
			{
				const unsigned width = LOWORD( lParam );
				const unsigned height = HIWORD( lParam );
				if ( width != getWidth() && height != getHeight() )
				{
					// #TODO: m_pGraphics->resize( width,
					//	height );
				}
			}
		}
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		// we want to avoid trying to resize the swapchain as the user does the 'rubber band' resize:
		s_bCurrentlyResizing = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		// here is the other place where you handle the swapchain resize after the user stops using the 'rubber-band' resize
		s_bCurrentlyResizing = false;
		break;
	}
	case WM_GETMINMAXINFO:
	{
		// sent when the size or position of the window is about to change - This is the very 1st message being sent to a window b4 WM_NCCREATE we want to prevent the window from being set too tiny
		auto info = reinterpret_cast<MINMAXINFO*>( lParam );
		info->ptMinTrackSize.x = 320;
		info->ptMinTrackSize.y = 200;
		break;
	}
	/// Menu messages
	case WM_COMMAND:
	{// sent when the user selects a command item from a menu, when a control sends a notification message to its parent window, or when an accelerator keystroke is translated
		const UINT id = LOWORD( wParam );
		const UINT bAccelerator = HIWORD( wParam ); // lParam is not used
		switch ( id )
		{
		case IDM_EDIT_CUT:
		{
			if ( editCopy() )
			{
				editDelete();
			}
			break;
		}
		case IDM_EDIT_COPY:
		{
			editCopy();
			break;
		}
		case IDM_EDIT_DELETE:
		{
			editDelete();
			break;
		}
		case IDM_EDIT_PASTE:
		{
			editPaste();
			break;
		}
		case IDM_HELP_HIDETHISMENU:
		{
			hideMenu();
			break;
		}
		case IDM_HELP_ABOUT:
		{
			m_pModalDialog.reset();
			m_pModalDialog = std::make_unique<Window>( 200, 100, "Dialog Box", MODAL_DIALOG_CLASS_NAME, Window::dialogProc, 350, 200, ColorBGRA{128, 128, 128}, nullptr, this );

			break;
		}
		case IDM_FILE_QUIT:
			PostQuitMessage( 0 );
			break;
		default:
			break;
		}
		break;
	}
	case WM_SYSCOMMAND:
	{
		// sent when the user chooses a command from the Window menu (formerly known as the system or control menu), or when the user chooses the maximize/minimize/close buttons, or when an accelerator keystroke is translated to be a syscommand
		// in WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter are used internally by the system
		switch( wParam & 0xFFF0 )
		{
		case SC_MINIMIZE:
			minimize();
			break;
		case SC_MAXIMIZE:
			restore();
			break;
		case SC_CLOSE:	// ALT-F4, or when X-ing out the window
			if ( displayMessageBoxYesNo( "Confirmation", "Are you sure you want to quit?" ) == IDOK )
			{
				PostQuitMessage( 0 );
			}
			break;
		}
		break;
	}
	case WM_INITMENUPOPUP:
	{
		// Sent when a drop-down menu or submenu is about to become active. This allows an application to modify the menu before it is displayed, without changing the entire menu.
#if defined _DEBUG && !defined NDEBUG
		using namespace std::string_literals;
		console.log( "Clipboard processing\n"s );
#endif
		processMenu( (HMENU) wParam );

		break;
	}
	/// clipboard messages
	case WM_RENDERFORMAT:
	{
		// support delayed clipboard rendering
		renderClipboardFormat( (unsigned) wParam );
		break;
	}
	case WM_RENDERALLFORMATS:
	{
		// support delayed clipboard rendering
		if ( !OpenClipboard( m_hWnd ) )
		{
			break;
		}
		if ( GetClipboardOwner() != m_hWnd )
		{
			break;
		}

		renderClipboardFormat();
		break;
	}
	case WM_DESTROYCLIPBOARD:
	{
		// destroy any resources that were set aside to support delayed clipboard rendering
		if ( s_pBoxLocalClip != nullptr )
		{
			LocalFree( s_pBoxLocalClip );
			s_pBoxLocalClip = nullptr;
		}
		break;
	}
	case WM_SIZECLIPBOARD:
	{
		break;
	}
	/// Keyboard messages
	case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		[[fallthrough]];
	case WM_SYSKEYDOWN:
	{
#ifndef FINAL_RELEASE
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
#endif
		if ( (lParam | 0x40000000) && !s_keyboard.isAutorepeatEnabled() ) // filter autorepeat
		{
			s_keyboard.onKeyPressed( static_cast<unsigned char>( wParam ) );	// wParam is key in decimal form
		}
		break;
	}
	case WM_KEYUP:
		[[fallthrough]];
	case WM_SYSKEYUP:
	{
#ifndef FINAL_RELEASE
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
#endif
		s_keyboard.onKeyReleased( static_cast<unsigned char>( wParam ) );
		break;
	}
	case WM_CHAR:
	{
#ifndef FINAL_RELEASE
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
#endif
		s_keyboard.onChar( static_cast<unsigned char>( wParam ) );
		break;
	}
	/// Mouse Messages
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		// cursorless exclusive gets first dibs
		if ( !m_bCursorEnabled )
		{
			if ( !s_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				s_mouse.onMouseEnterWindow();
				hideCursor();
			}
			break;
		}
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if ( pt.x >= 0 && pt.x < getWidth() && pt.y >= 0 && pt.y < getHeight() )
		{
			s_mouse.onMouseMove( pt.x, pt.y );
			if ( !s_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				s_mouse.onMouseEnterWindow();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			//if ( wParam & (MK_LBUTTON | MK_RBUTTON) )
			// if we're dragging while the cursor hot pixel is outside the windows bounds
			// keep getting mouse events even when it's outside the window
			if ( s_mouse.isLmbPressed() || s_mouse.isRmbPressed() )
			{
				s_mouse.onMouseMove( pt.x, pt.y );
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				s_mouse.onMouseLeaveWindow();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		SetForegroundWindow( hWnd );
		if ( !m_bCursorEnabled )
		{
			confineCursor();
			hideCursor();
		}
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onLmbPressed( pt.x, pt.y );
		break;
	}
	case WM_RBUTTONDOWN:
	{
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onRmbPressed( pt.x, pt.y );
		break;
	}
	case WM_LBUTTONUP:
	{
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onLmbReleased( pt.x, pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= getWidth() || pt.y < 0 || pt.y >= getHeight() )
		{
			ReleaseCapture();
			s_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		const POINTS pt = MAKEPOINTS( lParam );
		s_mouse.onRmbReleased( pt.x, pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= getWidth() || pt.y < 0 || pt.y >= getHeight() )
		{
			ReleaseCapture();
			s_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
#ifndef FINAL_RELEASE
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
#endif
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		s_mouse.onWheelDelta( pt.x, pt.y, delta );
		break;
	}
	case WM_INPUT:	// Raw Input Messages
	{
		if ( !s_mouse.isRawInputEnabled() )
		{
			break;
		}
		unsigned size = -1;
		// first get the size of the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, nullptr, &size, sizeof( RAWINPUTHEADER ) ) == -1 )
		{
			// bail msg processing if error
			break;
		}
		s_rawInputBuffer.resize( size );
		// read in the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ), RID_INPUT, s_rawInputBuffer.data(), &size, sizeof( RAWINPUTHEADER ) ) != size )
		{
			// bail msg processing if error
			break;
		}
		// process the raw input data
		auto &ri = reinterpret_cast<const RAWINPUT&>( *s_rawInputBuffer.data() );
		if ( ri.header.dwType == RIM_TYPEMOUSE && ( ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0 ) )
		{
			s_mouse.onRawDelta( ri.data.mouse.lLastX, ri.data.mouse.lLastY );
		}
		break;
	}
	case WM_TRAY_ICON:	// Tray Icon Messages
	{
		if ( lParam == WM_LBUTTONUP )
		{
			restore();
		}
		else if ( lParam == WM_RBUTTONDOWN )
		{
			POINT cursorPosScreenCoords;
			GetCursorPos( &cursorPosScreenCoords);
			SetForegroundWindow( hWnd );
			// spawn a popup menu on mouse hotspot coordinates - TrackPopupMenu blocks
			UINT bClicked = TrackPopupMenu( m_hTrayIconPopupMenu, TPM_RETURNCMD | TPM_NONOTIFY, cursorPosScreenCoords.x, cursorPosScreenCoords.y, 0, m_hWnd, nullptr );
			// send benign message to window to make sure the menu goes away.
			SendMessageW( m_hWnd, WM_NULL, 0, 0 );
			if ( bClicked == IDM_TRAY_ICON_EXIT )
			{
				deleteTrayIcon();
				PostQuitMessage( 0 );
				return 0;
			}
		}
		break;
	}
#pragma region unused_messages
	//case WM_NCHITTEST:
	//{
	//	// determine what part of the window the mouse cursor is on
	//	LRESULT uHitTest = DefWindowProcW( hWnd, WM_NCHITTEST, wParam, lParam );
	//	// or:
	//	//short int xPos = GET_X_LPARAM( lParam );
	//	//short int yPos = GET_Y_LPARAM( lParam );
	//	if ( uHitTest == HTCLIENT )
	//	{
	//		// client area
	//		return HTCAPTION;
	//	}
	//	else
	//	{
	//		// title bar or someplace else
	//		return uHitTest;
	//	}
	//}
#ifdef USE_GDIPLUS
	case WM_ERASEBKGND:	// called before WM_PAINT to update the background of the window client region, pass it on to DefWindowProc(), which uses the WNDCLASSEX::hbrBackground you selected
	{
		// called before WM_PAINT to update the background of the window client region
		// pass it on to DefWindowProc(), which uses the WNDCLASSEX::hbrBackground you selected
		break;
	}
	case WM_PAINT:
	{
#	if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Painting without DirectX\n"s );
#	endif
		PAINTSTRUCT ps;
		HDC hDc = BeginPaint( m_hWnd, &ps );

		LPRECT rect;
		GetClientRect( hWnd, &rect );
		bool bFlipFlop = true;
		HBRUSH hBrush = CreateSolidBrush( bFlipFlop ? col::Red : col::Green );
		FillRect( m_dcClientArea, rect, hBrush );

		EndPaint( hWnd, &ps );
		break;
	}
#endif // USE_GDIPLUS
#pragma endregion
	}//switch

	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK Window::dialogProc( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

	switch ( uMsg )
	{
	case WM_CREATE:
	{
#if defined _DEBUG && !defined NDEBUG
		console.print( "Modal Dialog Window created!" );
#endif
		break;
	}
	case WM_COMMAND:
	{
		const UINT id = LOWORD( wParam );
		switch ( id )
		{
			wchar_t itemName[BUTTON_TEXT_MAX_LENGTH];	// receives name of item to delete.
			// dialog box command ids
			case IDOK:
			{
				if ( GetDlgItemTextW( hWnd, IDOK, itemName, BUTTON_TEXT_MAX_LENGTH ) )
				{
					EndDialog( hWnd, wParam );
					return TRUE;
				}
				break;
			}
			case IDCANCEL:
			{
				if ( GetDlgItemTextW( hWnd, IDCANCEL, itemName, BUTTON_TEXT_MAX_LENGTH ) )
				{
					EndDialog( hWnd, wParam );
					return TRUE;
				}

				break;
			}
		}//switch
		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	default:
	{
		break;
	}
	}//switch

	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK Window::splashWindowProc( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

	switch ( uMsg )
	{
	case WM_CREATE:
	{
#if defined _DEBUG && !defined NDEBUG
		console.print( "Splash Window has been created!" );
#endif
		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	default:
	{
		break;
	}
	}//switch

	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

void CALLBACK Window::splashWindowTimerProc( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const unsigned idEvent,
	_In_ const DWORD time )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	PRINT_WIN_MESSAGE;
#endif

	if ( s_timerEvent == idEvent )
	{
#if defined _DEBUG && !defined NDEBUG
		console.print( "Timer about to be killed!" );
#endif
		DestroyWindow( hWnd );

		KillTimer( hWnd, idEvent );
	}
}

void Window::setFont( const std::string &fontName )
{
	NONCLIENTMETRICS ncm{};
	ncm.cbSize = sizeof( NONCLIENTMETRICS );
	SystemParametersInfoW( SPI_GETNONCLIENTMETRICS, sizeof( NONCLIENTMETRICS ), &ncm, 0u );
	HFONT hFont = CreateFontIndirectW( &ncm.lfMessageFont );
	hFont = CreateFontW( 13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, util::s2ws( fontName ).data() );
	SendMessageW( m_hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM( TRUE, 0 ) );
}

void Window::resize( const int width,
	const int height,
	const unsigned flags /* = SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE */ ) const noexcept
{
	SetWindowPos( m_hWnd, HWND_NOTOPMOST, getX(), getY(), width, height, flags );
	ASSERT_HRES_WIN32_IF_FAILED;
}

// WindowException
Window::WindowException::WindowException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

std::string Window::WindowException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* Window::WindowException::what() const noexcept
{
	return KeyException::what();
}

unsigned Window::getWindowStyles() const noexcept
{
	return ::GetWindowLongW( m_hWnd, GWL_STYLE );
}

unsigned Window::getWindowStylesEx() const noexcept
{
	return ::GetWindowLongW( m_hWnd, GWL_EXSTYLE );
}

void Window::setWindowStyles( const unsigned windowStyles ) const noexcept
{
	::SetWindowLongW( m_hWnd, GWL_STYLE, windowStyles );
}

void Window::setWindowStylesEx( const unsigned windowStylesEx ) const noexcept
{
	::SetWindowLongW( m_hWnd, GWL_EXSTYLE, windowStylesEx );
}

void Window::setBorderless() const noexcept
{
	::SetWindowLongPtrW( m_hWnd, GWL_STYLE, getWindowStyles() & ~( WS_BORDER | WS_DLGFRAME | WS_THICKFRAME ) );
	ASSERT_HRES_WIN32_IF_FAILED;

	::SetWindowLongPtrW( m_hWnd, GWL_EXSTYLE, getWindowStylesEx() & ~WS_EX_DLGMODALFRAME );
	ASSERT_HRES_WIN32_IF_FAILED;
}

void Window::setRedrawing( const bool bRedraw )
{
	int ret = ::LockWindowUpdate( bRedraw ? nullptr : m_hWnd );
	ASSERT( ret, "Could not lock window!" );
}

void Window::showMenu( const HMENU hMenu )
{
	// hide the menu bar, change styles and position and redraw
	setRedrawing( false );

	::SetMenu( m_hWnd, hMenu );
	ASSERT_HRES_WIN32_IF_FAILED;

	setRedrawing( true );
}

void Window::hideMenu()
{
	setRedrawing( false );

	::SetMenu( m_hWnd, nullptr );
	ASSERT_HRES_WIN32_IF_FAILED;

	setRedrawing( true );
}

const HMENU Window::getTopMenu() const noexcept
{
	return GetMenu( m_hWnd );
}

HWND Window::getDesktop() const noexcept
{
	// parentless windows end up under GetDesktopWindow() by default
	return GetDesktopWindow();
}

int Window::getX() const noexcept
{
	RECT rect;
	GetWindowRect( m_hWnd, &rect );
	// Don't use ASSERT_HRES_WIN32_IF_FAILED; on GetWindowRect
	return rect.left;
}

int Window::getY() const noexcept
{
	RECT rect;
	GetWindowRect( m_hWnd, &rect );
	return rect.top;
}

int Window::getWidth() const noexcept
{
	RECT rect;
	GetWindowRect( m_hWnd, &rect );
	return rect.right - rect.left;
}

int Window::getHeight() const noexcept
{
	RECT rect;
	GetWindowRect( m_hWnd, &rect );
	return rect.bottom - rect.top;
}

HWND Window::getConsoleHandle() const
{
	HWND ret = FindWindowA("ConsoleWindowClass", nullptr );
	if ( ret == 0 )
	{
		THROW_WINDOW_EXCEPTION( "No console has been allocated for this window." );
	}
	return ret;
}

const Window::WindowClass& Window::getWindowClass() noexcept
{
	return m_windowClass;
}

#pragma warning( disable : 4312 )
void WINAPI Window::processMenu( HMENU hMenu )
{
	int nMenuItems = GetMenuItemCount( hMenu );
	unsigned flags;
	PLABELBOX pBox = ( m_hWnd == nullptr ) ?
		nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );

	for ( int i = 0; i < nMenuItems; ++i )
	{
		unsigned id = GetMenuItemID( hMenu, i );

		switch ( id )
		{
		case IDM_EDIT_CUT:
		case IDM_EDIT_COPY:
		case IDM_EDIT_DELETE:
		{
			if ( pBox == nullptr || ~pBox->fSelected )
			{
				flags = MF_BYCOMMAND | MF_GRAYED;
			}
			else if ( pBox->fEdit )
			{
				flags = ( id != IDM_EDIT_DELETE && pBox->ichSel == pBox->ichCaret ) ?
					MF_BYCOMMAND | MF_GRAYED :
					MF_BYCOMMAND | MF_ENABLED;
			}
			else
			{
				flags = MF_BYCOMMAND | MF_ENABLED;
			}

			EnableMenuItem( hMenu, id, flags );
			break;
		}
		case IDM_EDIT_PASTE:
		{
			if ( pBox != nullptr && pBox->fEdit )
			{
				// clipboard CF_ formats: https://learn.microsoft.com/en-us/windows/win32/dataxchg/standard-clipboard-formats
				const UINT ansiText = CLIPBOARD_TEXT_FORMAT;
				const bool bClipboardAvailable = IsClipboardFormatAvailable( ansiText ) ?
					MF_BYCOMMAND | MF_ENABLED :
					MF_BYCOMMAND | MF_GRAYED;

				EnableMenuItem( hMenu, id, bClipboardAvailable );
			}
			else
			{
				const bool bClipboardAvailable = IsClipboardFormatAvailable( m_clipboardFormats[0] );

				EnableMenuItem( hMenu, id, bClipboardAvailable );
			}

			break;
		}
		default:
			break;
		}
	}
}
#pragma warning( default : 4312 )

#pragma warning( disable : 4312 )
bool Window::editCopy( const UINT format )
{
	HGLOBAL hClipboard;

	if ( !IsClipboardFormatAvailable( format ) )
	{
		return false;
	}

	// open the clipboard
	if ( !OpenClipboard( m_hWnd ) )
	{
		return false;
	}
	ASSERT_HRES_WIN32_IF_FAILED;

	EmptyClipboard();
	ASSERT_HRES_WIN32_IF_FAILED;

	// get a pointer to the structure for the selected label
	PLABELBOX pBox = ( m_hWnd == nullptr ) ?
		nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );
	ASSERT_HRES_WIN32_IF_FAILED;

	if ( format == CF_TEXT || format == CF_UNICODETEXT )
	{
		int charStartPos;
		int charEndPos;
		int nChars;

		// if text is selected copy it
		if ( pBox->fEdit )
		{
			if ( pBox->ichSel == pBox->ichCaret )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			if ( pBox->ichSel < pBox->ichCaret )
			{
				charStartPos = pBox->ichSel;
				charEndPos = pBox->ichCaret;
			}
			else
			{
				charStartPos = pBox->ichCaret;
				charEndPos = pBox->ichSel;
			}
			nChars = charEndPos - charStartPos;

			// allocate a global memory object for the text
			/*
			* Global|LocalALloc are remnants from 16bit era and on 32 bit systems are doing essentially the same thing and they call HeapAlloc.
			* use HeapCreate & Alloc & Lock & Unlock & Free instead.
			constexpr unsigned heapCanGrowInSize = 0u;
			HANDLE h = HeapCreate( HEAP_NO_SERIALIZE, ( nChars + 1 ) * sizeof( TCHAR ), heapCanGrowInSize );
			*/
			hClipboard = GlobalAlloc( GMEM_MOVEABLE, ( nChars + 1 ) * sizeof( TCHAR ) );
			ASSERT_HRES_WIN32_IF_FAILED;

			if ( hClipboard == nullptr )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			// lock the handle and copy the text to the buffer
			wchar_t *pCopiedStr = static_cast<wchar_t*>( GlobalLock( hClipboard ) );
			ASSERT_HRES_WIN32_IF_FAILED;

			memcpy( pCopiedStr, &pBox->atchLabel[charStartPos], nChars * sizeof( TCHAR ) );
			pCopiedStr[nChars] = (TCHAR) 0;	// for the null terminator

			GlobalUnlock( hClipboard );
			ASSERT_HRES_WIN32_IF_FAILED;

			// place the handle on the clipboard
			SetClipboardData( format, hClipboard );
			ASSERT_HRES_WIN32_IF_FAILED;
		}
		else
		{
			// if no text is selected, the label as a whole is copied
			s_pBoxLocalClip = (PLABELBOX) LocalAlloc( LMEM_FIXED, sizeof( LABELBOX ) );
			ASSERT_HRES_WIN32_IF_FAILED;

			if ( s_pBoxLocalClip == nullptr )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			memcpy( s_pBoxLocalClip, pBox, sizeof( LABELBOX ) );
			s_pBoxLocalClip->fSelected = FALSE;
			s_pBoxLocalClip->fEdit = FALSE;

			// place a registered clipboard format, the owner-display format and the CF_TEXT format on the clipboard using delayed rendering (only when and if they are needed)
			// to handle the "when they are needed" we must process the WM_RENDERFORMAT and WM_RENDERALLFORMATS messages
			SetClipboardData( CF_OWNERDISPLAY, nullptr );
			ASSERT_HRES_WIN32_IF_FAILED;
			SetClipboardData( format, nullptr );
			ASSERT_HRES_WIN32_IF_FAILED;
		}
	}

	CloseClipboard();
	ASSERT_HRES_WIN32_IF_FAILED;
	return true;
}
#pragma warning( default : 4312 )

#pragma warning( disable : 4312 )
bool Window::editPaste( const unsigned format )
{
	if ( !IsClipboardFormatAvailable( format ) )
	{
		return false;
	}

	// get a pointer to the structure for the selected label
	PLABELBOX pBox = ( m_hWnd == nullptr ) ?
		nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );
	ASSERT_HRES_WIN32_IF_FAILED;

	// if the application is in edit mode get the clipboard text
	if ( format == CF_TEXT || format == CF_UNICODETEXT )
	{
		if ( pBox != nullptr && pBox->fEdit )
		{
			if ( !OpenClipboard( m_hWnd ) )
			{
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			HGLOBAL hClipboard = GetClipboardData( format );
			ASSERT_HRES_WIN32_IF_FAILED;
			if ( hClipboard == nullptr )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			wchar_t *pCopiedStr = static_cast<wchar_t*>( GlobalLock( hClipboard ) );
			ASSERT_HRES_WIN32_IF_FAILED;
			if ( pCopiedStr == nullptr )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			// #TODO: call the application-defined ReplaceSelection function to insert the text and repaint the window
			//ReplaceSelection( hwndSelected,
			//	pBox,
			//	pCopiedStr );

			GlobalUnlock( hClipboard );
			ASSERT_HRES_WIN32_IF_FAILED;
		}
	}

	CloseClipboard();
	ASSERT_HRES_WIN32_IF_FAILED;
	return true;
}
#pragma warning( default : 4312 )

#pragma warning( disable : 4312 )
void Window::renderClipboardFormat( unsigned format )
{
	HGLOBAL hClipboard;

	// get a pointer to the structure for the selected label
	PLABELBOX pBox = ( m_hWnd == nullptr ) ?
		nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );
	ASSERT_HRES_WIN32_IF_FAILED;

	if ( format == CF_TEXT || format == CF_UNICODETEXT )
	{
		// allocate a buffer for the text
		int nChars = s_pBoxLocalClip->cchLabel;

		hClipboard = GlobalAlloc( GMEM_MOVEABLE, (nChars + 1) * sizeof( TCHAR ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		// copy the text from the s_pBoxLocalClip
		wchar_t *pCopiedStr = static_cast<wchar_t*>( GlobalLock( hClipboard ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		memcpy( pCopiedStr, s_pBoxLocalClip->atchLabel, nChars * sizeof( TCHAR ) );
		pCopiedStr[nChars] = (TCHAR)0;

		GlobalUnlock( hClipboard );
		ASSERT_HRES_WIN32_IF_FAILED;

		// place the handle on the clipboard
		SetClipboardData( format, hClipboard );
	}
	else
	{
		hClipboard = GlobalAlloc( GMEM_MOVEABLE, sizeof( LABELBOX ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		pBox = static_cast<PLABELBOX>( GlobalLock( hClipboard ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		memcpy( pBox, s_pBoxLocalClip, sizeof( LABELBOX ) );

		GlobalUnlock( hClipboard );
		ASSERT_HRES_WIN32_IF_FAILED;

		SetClipboardData( format, hClipboard );
		ASSERT_HRES_WIN32_IF_FAILED;
	}

	CloseClipboard();
	ASSERT_HRES_WIN32_IF_FAILED;
}
#pragma warning( default : 4312 )

void Window::editDelete()
{
	// #TODO:
}