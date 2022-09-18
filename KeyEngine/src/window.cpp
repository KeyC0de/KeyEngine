#include <sstream>
#include <string>
#include <windowsx.h>
#include "window.h"
#include "graphics.h"
#include "imgui_impl_win32.h"
#include "utils.h"
#include "os_utils.h"
#include "math_utils.h"
#include "console.h"
#include "assertions_console.h"
#include "graphics_mode.h"
#include "../resource.h"

//#define WM_TRAY_ICON			10001
//#define ID_TRAY_APP_ICON		20001
//#define ID_TRAY_ICON_EXIT		20002


Window::WindowClass::WindowClass( const std::string &name )
	:
	m_name{name}
{
	HINSTANCE hInstance = THIS_INSTANCE;
	ASSERT( hInstance != nullptr, "HINSTANCE is null!" );

	WNDCLASSEX wc{};
	wc.cbSize = sizeof WNDCLASSEX;
	wc.style = CS_OWNDC
			//| CS_DBLCLKS
			| CS_HREDRAW | CS_VREDRAW;	// send WM_SIZE message when either the height or the width of the client area are changed
	wc.lpfnWndProc = windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;	// extra bytes
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursorW( hInstance,
		MAKEINTRESOURCEW( IDC_HOMM_INSPIRED ) );
	ASSERT_HRES_WIN32_IF_FAILED;
	wc.hIcon = static_cast<HICON>( LoadImageW( hInstance,
		MAKEINTRESOURCEW( IDI_ICON_APP ),
		IMAGE_ICON,
		64,
		43,
		0u ) );
	ASSERT_HRES_WIN32_IF_FAILED;
	wc.hIconSm = static_cast<HICON>( LoadImageW( hInstance,
		MAKEINTRESOURCEW( IDI_ICON_APP_SMALL ),
		IMAGE_ICON,
		32,
		22,
		0u ) );
	ASSERT_HRES_WIN32_IF_FAILED;
	wc.hbrBackground = CreateSolidBrush( RGB( 255, 255, 255 ) );
	wc.lpszMenuName = nullptr;
	const std::wstring className = util::s2ws( name );
	wc.lpszClassName = className.data();
	m_classAtom = RegisterClassExW( &wc );
	ASSERT_HRES_WIN32_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::instance();
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
	UnregisterClassW( util::s2ws( m_name ).data(),
		GetModuleHandleW( nullptr ) );
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
	std::swap( *this,
		tmp );
	return *this;
}

const std::string& Window::WindowClass::getName() noexcept
{
	return m_name;
}

Window::Window( const int width,
	const int height,
	const char *name )
	:
	m_windowClass{"KeyEngine_Window_Class"},
	m_width(width),
	m_height(height),
	m_name{name}
{
	RECT rect{0, 0, width, height};
	BOOL ret = AdjustWindowRect( &rect,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		FALSE );
	if ( !ret )
	{
		throwWindowException( "Failed to adjust window rectangle" );
	}

	uint32_t windowExStyles = WS_EX_OVERLAPPEDWINDOW;
	uint32_t windowStyles = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_OVERLAPPEDWINDOW;	// disable both maximizing and resizing
	const std::wstring className = util::s2ws( m_windowClass.getName() );
	const std::wstring windowName = util::s2ws( name );
	m_hWnd = CreateWindowExW( windowExStyles,
		className.data(),
		windowName.data(),
		windowStyles,
		200,
		200,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		//HWND_DESKTOP,	// parent
		nullptr,//LoadMenu( hInstance, MAKEINTRESOURCEW( IDR_MENU_APP ) ), // menu
		THIS_INSTANCE,
		this );
	ASSERT_HRES_WIN32_IF_FAILED;
	if ( !m_hWnd )
	{
		throwWindowException( "Failed to create Window" );
	}

	// mouse raw input
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01u;	// mouse page
	rid.usUsage = 0x02u;		// mouse usage
	rid.dwFlags = 0u;
	rid.hwndTarget = nullptr;
	ret = RegisterRawInputDevices( &rid,
		1u,
		sizeof RAWINPUTDEVICE );
	ASSERT_HRES_WIN32_IF_FAILED;

	m_info.cbSize = sizeof WINDOWINFO;
	GetWindowInfo( m_hWnd,
		&m_info );
	ASSERT_HRES_WIN32_IF_FAILED;

	// set title
	static const short maxTitleCharSize = 128i16;
	wchar_t title[maxTitleCharSize];
	GetWindowTextW( m_hWnd,
		title,
		maxTitleCharSize );
	m_title = util::ws2s( title );

	// (keyboard) accelerators are keystrokes or a combination of keystrokes that generate a WM_COMMAND or WM_SYSCOMMAND message for an application
	m_hAcceleratorTable = LoadAcceleratorsW( THIS_INSTANCE,
		MAKEINTRESOURCEW( ID_ACCEL_TABLE_APP ) );
	ASSERT_HRES_WIN32_IF_FAILED;

	// register other clipboard formats to handle:
	// we assume that atchTemp can contain the format name and a null-terminator, otherwise it is truncated
	wchar_t unicodeClipboardFormat[32];
	LoadStringW( THIS_INSTANCE,
		IDS_CLIPBOARD_UNICODE_FORMAT,
		unicodeClipboardFormat,
		sizeof( unicodeClipboardFormat) / sizeof( WCHAR ) );
	m_clipboardFormats[0] = RegisterClipboardFormatW( unicodeClipboardFormat );
	ASSERT_HRES_WIN32_IF_FAILED;

	// Tray icon setup
	//setupNotifyIconData();

	// Initialize ImGui
	if constexpr( gph_mode::get() == gph_mode::_3D )
	{
		ImGui_ImplWin32_Init( m_hWnd );
	}

	// Graphics for this Window
	m_pGraphics = std::make_unique<Graphics>( m_hWnd,
		width,
		height );

	// Display window
	ret = UpdateWindow( m_hWnd );
	if ( ret == FALSE )
	{
		throwWindowException( "Failed to update the client area of the window." );
	}
	ret = ShowWindow( m_hWnd,
		SW_SHOWDEFAULT );
}

Window::~Window()
{
	ReleaseDC( m_hWnd,
		m_dc );
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		ImGui_ImplWin32_Shutdown();
	}
	DestroyWindow( m_hWnd );
}

Window::Window( Window &&rhs ) noexcept
	:
	m_windowClass{std::move( rhs.m_windowClass )},
	m_bCursorEnabled{std::move( rhs.m_bCursorEnabled )},
	m_width{rhs.m_width},
	m_height{rhs.m_height},
	m_name{std::move( rhs.m_name )},
	m_hWnd{std::move( rhs.m_hWnd )},
	m_pGraphics{std::move( rhs.m_pGraphics )},
	m_dc{rhs.m_dc},
	m_info{std::move( rhs.m_info )},
	m_clipboardFormats{std::move( rhs.m_clipboardFormats )}
{
	m_keyboard = std::move( rhs.m_keyboard );
	rhs.m_keyboard = {};
	m_mouse = std::move( rhs.m_mouse );
	rhs.m_mouse = {};
	m_rawInputBuffer = std::move( rhs.m_rawInputBuffer );
	rhs.m_rawInputBuffer = {};
}

Window& Window::operator=( Window &&rhs ) noexcept
{
	std::swap( *this,
		rhs );
	return *this;
}

void Window::setEnable( const bool b )
{
	EnableWindow( m_hWnd,
		b ? TRUE :
			FALSE );
}

void Window::setOnTop()
{
	SetWindowPos( m_hWnd,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE );
	// redraw to prevent the window blank
	RedrawWindow( m_hWnd,
		nullptr,
		nullptr,
		RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN );
}

const HWND Window::setFocus()
{
	HWND previouslyFocusedWindow = SetFocus( m_hWnd );
	return previouslyFocusedWindow;
}

const HWND Window::getParent() const noexcept
{
	return GetParent( m_hWnd );
}

void Window::setTitle( const std::string &title )
{
	SetWindowTextW( m_hWnd,
		util::s2ws( title ).data() );
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

void Window::configureDc()
{
	m_dc = GetWindowDC( m_hWnd );
}

void Window::displayMessageBox( const std::string &title,
	const std::string &message ) const
{
	MessageBoxW( m_hWnd,
		util::s2ws( message ).c_str(),
		util::s2ws( title ).c_str(),
		MB_OKCANCEL );
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
	ShowWindow( m_hWnd,
		SW_HIDE );
}

void Window::restore()
{
	ShowWindow( m_hWnd,
		SW_SHOW );
	SetForegroundWindow( m_hWnd );
}

//void Window::setupNotifyIconData()
//{
//	ZeroMemory( &m_notifyIconData, sizeof NOTIFYICONDATA );
//	m_notifyIconData.cbSize = sizeof NOTIFYICONDATA;
//	m_notifyIconData.hWnd = m_hWnd;
//	m_notifyIconData.uID = ID_TRAY_APP_ICON;	// stored in the wParam data of the uMsg
//	m_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;	// popup menu style
//	m_notifyIconData.uCallbackMessage = WM_TRAY_ICON;
//	m_notifyIconData.hIcon = LoadIconW( GetModuleHandleW( nullptr ),
//		MAKEINTRESOURCEW( IDI_ICON_TRAY ) );
//	std::wstring szTrayIconTooltipText{L"KeyEngine is relaxing in the System Tray."};
//	wcsncpy( m_notifyIconData.szTip,
//		szTrayIconTooltipText.c_str(),
//		szTrayIconTooltipText.size() );
//}

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
		// check for quit because peekmessage does not signal this via return val
		if ( msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY )
		{
			return (int)msg.wParam;
		}
		if ( !TranslateAcceleratorW( m_hWnd, m_hAcceleratorTable, &msg ) )
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

HDC Window::getDc() const noexcept
{
	return m_dc;
}

WINDOWINFO Window::getInfo() const noexcept
{
	return m_info;
}

Keyboard& Window::keyboard() noexcept
{
	return m_keyboard;
}

Mouse& Window::mouse() noexcept
{
	return m_mouse;
}

const Window::WindowClass& Window::getWindowClass() noexcept
{
	return m_windowClass;
}

bool Window::isDescendantOf( const HWND targethWnd,
	const HWND parent ) noexcept
{
	return IsChild( parent, targethWnd ) == 0 ? false :
		true;
}

void saveClipboardTextAsVar()
{
	HANDLE hClipboard;
	char *clipboardText;
	
	if ( OpenClipboard( nullptr ) )
	{
		hClipboard = GetClipboardData(CF_TEXT );
		clipboardText = (char*)hClipboard;
#if defined _DEBUG && !defined NDEBUG
		KeyConsole& console = KeyConsole::instance();
		console.print( "Clipboard copied text: " + std::string{clipboardText} + "\n" );
#endif
		CloseClipboard();
	}
}

void Window::confineCursor() noexcept
{
	RECT rect;
	GetClientRect( m_hWnd,
		&rect );
	MapWindowPoints( m_hWnd,
		nullptr,
		reinterpret_cast<POINT*>( &rect ),
		2u );
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
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Window::disableImGuiMouse() noexcept
{
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

LRESULT CALLBACK Window::windowProc( const HWND hWnd,
	const unsigned uMsg,
	const WPARAM wParam,
	const LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::instance();
	console.log( "windowProc() entered\n" );
#endif

	if ( uMsg == WM_NCCREATE )
	{
#if defined _DEBUG && !defined NDEBUG
		console.log( "windowProc() WM_NCCREATE message\n" );
#endif
		const CREATESTRUCTW *const cs = (CREATESTRUCTW*) lParam;
		Window *this_wnd = reinterpret_cast<Window*>( cs->lpCreateParams );

		this_wnd->m_hWnd = hWnd;

		SetWindowLongPtrW( hWnd,
			GWLP_USERDATA,
			(LONG_PTR) this_wnd );
		SetWindowLongPtrW( hWnd,
			GWLP_WNDPROC,
			(LONG_PTR) &Window::windowProcDelegate );
		return this_wnd->windowProcDelegate( hWnd,
			uMsg,
			wParam,
			lParam );
	}
	// if we get a msg before the WM_NCCREATE msg handle it with the default windowProc
	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}

LRESULT CALLBACK Window::windowProcDelegate( const HWND hWnd,
	const unsigned uMsg,
	const WPARAM wParam,
	const LPARAM lParam )
{
	Window *this_wnd = reinterpret_cast<Window*>( GetWindowLongPtrW( hWnd,
		GWLP_USERDATA ) );
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		return this_wnd->windowProc_impl3d( hWnd,
			uMsg,
			wParam,
			lParam );
	}
	else
	{
		return this_wnd->windowProc_impl2d( hWnd,
			uMsg,
			wParam,
			lParam );
	}
}

LRESULT Window::windowProc_impl2d( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
	// Predefined Windows messages
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
		// clear keyboard button states when window loses focus
		//	to prevent keyboard input getting "stuck"
		m_keyboard.clearKeyStates();
		break;
	}
	case WM_ACTIVATEAPP:
	// Sent to the top level window belonging to a different application than the active window is about to be activated as well as the top level window of this application
	case WM_ACTIVATE:
	{// message sent to the window being activated and to the window being deactivated
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::instance();
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
//		Shell_NotifyIconW( NIM_ADD,
//			&m_notifyIconData );
		break;
	}
	/// Keyboard Messages
	case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		[[fallthrough]];
	case WM_SYSKEYDOWN:
	{
		if ( (lParam | 0x40000000) && !m_keyboard.isAutorepeatEnabled() ) // filter autorepeat
		{
			m_keyboard.onKeyPressed( static_cast<unsigned char>( wParam ) );
		}
		break;
	}
	case WM_KEYUP:
		[[fallthrough]];
	case WM_SYSKEYUP:
	{
		m_keyboard.onKeyReleased( static_cast<unsigned char>( wParam ) );
		break;
	}
	case WM_CHAR:
	{
		m_keyboard.onChar( static_cast<unsigned char>( wParam ) );
		break;
	}
	/// Mouse Messages
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		// cursorless exclusive gets first dibs
		if ( !m_bCursorEnabled )
		{
			if ( !m_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				m_mouse.onMouseEnterWindow();
				hideCursor();
			}
			break;
		}
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if ( pt.x >= 0 && pt.x < m_width && pt.y >= 0 && pt.y < m_height )
		{
			m_mouse.onMouseMove( pt.x,
				pt.y );
			if ( !m_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				m_mouse.onMouseEnterWindow();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			//if ( wParam & (MK_LBUTTON | MK_RBUTTON) )
			// if we're dragging while the cursor hot pixel is outside the windows bounds
			// keep getting mouse events even when it's outside the window
			if ( m_mouse.isLmbPressed() || m_mouse.isRmbPressed() )
			{
				m_mouse.onMouseMove( pt.x,
					pt.y );
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				m_mouse.onMouseLeaveWindow();
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
		m_mouse.onLmbPressed( pt.x,
			pt.y );
		break;
	}
	case WM_RBUTTONDOWN:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onRmbPressed( pt.x,
			pt.y );
		break;
	}
	case WM_LBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onLmbReleased( pt.x,
			pt.y );
		// release mouse if outside of a window
		if ( pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height )
		{
			ReleaseCapture();
			m_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onRmbReleased( pt.x,
			pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height )
		{
			ReleaseCapture();
			m_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		m_mouse.onWheelDelta( pt.x,
			pt.y,
			delta );
		break;
	}
	/// Raw Input Messages
	case WM_INPUT:
	{
		if ( !m_mouse.isRawInputEnabled() )
		{
			break;
		}
		unsigned size = -1;
		// first get the size of the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ),
			RID_INPUT,
			nullptr,
			&size,
			sizeof( RAWINPUTHEADER ) ) == -1 )
		{
			// bail msg processing if error
			break;
		}
		m_rawInputBuffer.resize( size );
		// read in the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ),
			RID_INPUT,
			m_rawInputBuffer.data(),
			&size,
			sizeof( RAWINPUTHEADER ) ) != size )
		{
			// bail msg processing if error
			break;
		}
		// process the raw input data
		auto &ri = reinterpret_cast<const RAWINPUT&>( *m_rawInputBuffer.data() );
		if ( ri.header.dwType == RIM_TYPEMOUSE &&
			( ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0 ) )
		{
			m_mouse.onRawDelta( ri.data.mouse.lLastX,
				ri.data.mouse.lLastY );
		}
		break;
	}
	}//switch

	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}

LRESULT Window::windowProc_impl3d( _In_ const HWND hWnd,
	_In_ const unsigned uMsg,
	_In_ const WPARAM wParam,
	_In_ const LPARAM lParam )
{
	// ImGui windowProc messages
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) )
	{
		return true;
	}
	const auto &imGuiIoContext = ImGui::GetIO();

	static bool bMinimized = false;
	static bool bCurrentlyResizing = false;

	// custom messages (create with RegisterWindowMessage)

	// Predefined Windows messages
	switch( uMsg )
	{
	case WM_CREATE:
	{
//		// Initialization. Controls - child windows, set default values for controls
//#if defined _DEBUG && !defined NDEBUG
//		auto &console = KeyConsole::instance();
//		using namespace std::string_literals;
//		console.log( "'WM_CREATE' : window created\n"s );
//#endif
//		//m_splash = std::make_unique<SplashWindow>( hWnd,
//		//	THIS_INSTANCE,
//		//	IDB_SPLASH,
//		//	std::make_pair( 256, 256 ) );
//		//m_splash->messageLoop();
//
//		m_hTrayIconPopupMenu = CreatePopupMenu();
//		int ret = AppendMenuW( m_hTrayIconPopupMenu,
//			MF_STRING,
//			ID_TRAY_ICON_EXIT,
//			L"Exit the Application." );
//		ASSERT_HRES_WIN32_IF_FAILED;
		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	case WM_KILLFOCUS:
	{
		// clear keyboard button states when window loses focus
		//	to prevent keyboard input getting "stuck"
		m_keyboard.clearKeyStates();
		break;
	}
	case WM_ACTIVATEAPP:
	// Sent to the top level window belonging to a different application than the active window is about to be activated as well as the top level window of this application
	case WM_ACTIVATE:
	{// message sent to the window being activated and to the window being deactivated
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::instance();
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
//		Shell_NotifyIconW( NIM_ADD,
//			&m_notifyIconData );
		break;
	}
#if defined _DEBUG && !defined NDEBUG
	case WM_SHOWWINDOW:
	{
		// sent to a window when the window is about to be hidden or shown
		auto &console = KeyConsole::instance();
		if ( wParam == TRUE )
		{
			console.log( "Window " + getTitle() + " is visible.\n" );
		}
		else
		{
			console.log( "Window " + getTitle() + " is hidden.\n" );
		}
		//return 0;
		break;
	}
#endif
	/// resizing
	case WM_SIZE:
	{
		if ( wParam == SIZE_MINIMIZED )
		{
			// the window was minimized
			if ( !bMinimized )
			{
				bMinimized = true;
				// #TODO: suspend the application at this point so that we don't waste resources
			}
		}
		else if ( bMinimized )
		{
			// the window was minimized and is now restored (resume from suspend)
			bMinimized = false;
		}
		else if ( !bCurrentlyResizing )
		{
			// handle the swapchain resize for maximize or unmaximize
			// another option would be to prevent window resizing and iInstead resize by ingame option - choosing resolution
			m_width = LOWORD( lParam );
			m_height = HIWORD( lParam );
			if ( m_pGraphics )
			{
				m_pGraphics->resize( m_width,
					m_height );
			}
		}
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		// we want to avoid trying to resize the swapchain as the user does the 'rubber band' resize:
		bCurrentlyResizing = true;
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		// here is the other place where you handle the swapchain resize after the user stops using the 'rubber-band' 
		bCurrentlyResizing = false;
		break;
	}
	case WM_GETMINMAXINFO:
	{
		// we want to prevent the window from being set too tiny:
		auto info = reinterpret_cast<MINMAXINFO*>( lParam );
		info->ptMinTrackSize.x = 320;
		info->ptMinTrackSize.y = 200;
		break;
	}
	/// Menu messages
	case WM_COMMAND:
	{
	 // sent when the user selects a command item from a menu, when a control sends a notification message to its parent window, or when an accelerator keystroke is translated
		UINT id = LOWORD( wParam );
		UINT bAccelerator = HIWORD( wParam ); // lParam is not used
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

		//case IDM_HELP_ABOUT:
		//{
		//	m_pDialogAbout.reset();
		//	m_pDialogAbout = std::make_unique<Dialog>( L"KeyEngine_Dialog_Class" );
		//	HWND hWndDialogAbout = CreateWindowExW( WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
		//		L"KeyEngine_Dialog_Class",
		//		L"Dialog Box",
		//		WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
		//		100,
		//		100,
		//		200,
		//		150,
		//		nullptr,
		//		nullptr,
		//		THIS_INSTANCE,
		//		nullptr );
		//	ASSERT_HRES_WIN32_IF_FAILED;
		//	m_pDialogAbout->setHwnd( hWndDialogAbout );
		//	break;
		//}
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
		// sent when the user chooses a command from the Window menu (formerly known as the system or control menu) or when the user chooses the maximize/minimize/close buttons
		// in WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter are used internally by the system
		switch( wParam & 0xFFF0 )
		{
		case SC_MINIMIZE:
			minimize();
			break;
		case SC_MAXIMIZE:
			restore();
			break;
		}
		break;
	}
	case WM_INITMENUPOPUP:
	{
		// Sent when a drop-down menu or submenu is about to become active. This allows an application to modify the menu before it is displayed, without changing the entire menu.
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::instance();
		using namespace std::string_literals;
		console.log( "Clipboard processing\n"s );
#endif
		menuProc( (HMENU) wParam );

		break;
	}
	// clipboard messages
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
		if ( m_pBoxLocalClip != nullptr )
		{
			LocalFree( m_pBoxLocalClip );
			m_pBoxLocalClip = nullptr;
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
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
		if ( (lParam | 0x40000000) && !m_keyboard.isAutorepeatEnabled() ) // filter autorepeat
		{
			m_keyboard.onKeyPressed( static_cast<unsigned char>( wParam ) );
		}
		break;
	}
	case WM_KEYUP:
		[[fallthrough]];
	case WM_SYSKEYUP:
	{
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
		m_keyboard.onKeyReleased( static_cast<unsigned char>( wParam ) );
		break;
	}
	case WM_CHAR:
	{
		// stifle this keyboard message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureKeyboard )
		{
			break;
		}
		m_keyboard.onChar( static_cast<unsigned char>( wParam ) );
		break;
	}
	/// Mouse Messages
	case WM_MOUSEMOVE:
	{
		const POINTS pt = MAKEPOINTS( lParam );
		// cursorless exclusive gets first dibs
		if ( !m_bCursorEnabled )
		{
			if ( !m_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				m_mouse.onMouseEnterWindow();
				hideCursor();
			}
			break;
		}
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		// in client region -> log move, and log enter + capture mouse (if not previously in window)
		if ( pt.x >= 0 && pt.x < m_width && pt.y >= 0 && pt.y < m_height )
		{
			m_mouse.onMouseMove( pt.x,
				pt.y );
			if ( !m_mouse.isInWindow() )
			{
				SetCapture( hWnd );
				m_mouse.onMouseEnterWindow();
			}
		}
		// not in client -> log move / maintain capture if button down
		else
		{
			//if ( wParam & (MK_LBUTTON | MK_RBUTTON) )
			// if we're dragging while the cursor hot pixel is outside the windows bounds
			// keep getting mouse events even when it's outside the window
			if ( m_mouse.isLmbPressed() || m_mouse.isRmbPressed() )
			{
				m_mouse.onMouseMove( pt.x,
					pt.y );
			}
			// button up -> release capture / log event for leaving
			else
			{
				ReleaseCapture();
				m_mouse.onMouseLeaveWindow();
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
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onLmbPressed( pt.x,
			pt.y );
		break;
	}
	case WM_RBUTTONDOWN:
	{
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onRmbPressed( pt.x,
			pt.y );
		break;
	}
	case WM_LBUTTONUP:
	{
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onLmbReleased( pt.x,
			pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height )
		{
			ReleaseCapture();
			m_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		const POINTS pt = MAKEPOINTS( lParam );
		m_mouse.onRmbReleased( pt.x,
			pt.y );
		// release mouse if outside of window
		if ( pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height )
		{
			ReleaseCapture();
			m_mouse.onMouseLeaveWindow();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		// stifle this mouse message if imgui wants to capture
		if ( imGuiIoContext.WantCaptureMouse )
		{
			break;
		}
		const POINTS pt = MAKEPOINTS( lParam );
		const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
		m_mouse.onWheelDelta( pt.x,
			pt.y,
			delta );
		break;
	}
	/// Raw Input Messages
	case WM_INPUT:
	{
		if ( !m_mouse.isRawInputEnabled() )
		{
			break;
		}
		unsigned size = -1;
		// first get the size of the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ),
			RID_INPUT,
			nullptr,
			&size,
			sizeof( RAWINPUTHEADER ) ) == -1 )
		{
			// bail msg processing if error
			break;
		}
		m_rawInputBuffer.resize( size );
		// read in the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ),
			RID_INPUT,
			m_rawInputBuffer.data(),
			&size,
			sizeof( RAWINPUTHEADER ) ) != size )
		{
			// bail msg processing if error
			break;
		}
		// process the raw input data
		auto &ri = reinterpret_cast<const RAWINPUT&>( *m_rawInputBuffer.data() );
		if ( ri.header.dwType == RIM_TYPEMOUSE &&
			( ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0 ) )
		{
			m_mouse.onRawDelta( ri.data.mouse.lLastX,
				ri.data.mouse.lLastY );
		}
		break;
	}
	/// Tray icons
	//case WM_TRAY_ICON:
//	{
//		if ( lParam == WM_LBUTTONUP )
//		{
//			restore();
//		}
//		else if ( lParam == WM_RBUTTONDOWN )
//		{
//			POINT curPoint;
//			GetCursorPos( &curPoint );
//			SetForegroundWindow( hWnd );
//			// spawn a popup menu on mouse hotspot coordinates - TrackPopupMenu blocks
//			UINT clicked = TrackPopupMenu( m_hTrayIconPopupMenu,
//				TPM_RETURNCMD | TPM_NONOTIFY,
//				curPoint.x,
//				curPoint.y,
//				0,
//				m_hWnd,
//				nullptr );
//			// send benign message to window to make sure the menu goes away.
//			SendMessageW( m_hWnd,
//				WM_NULL,
//				0,
//				0 );
//			if ( clicked == ID_TRAY_ICON_EXIT )
//			{
//				Shell_NotifyIconW( NIM_DELETE,
//					&m_notifyIconData );
//				PostQuitMessage( 0 );
//				return 0;
//			}
//		}
//		break;
//	}
	//case WM_NCHITTEST:	// drags a window using its client area
	//{
	//	// determine what part of the window the mouse cursor is on
	//	LRESULT uHitTest = DefWindowProcW( hWnd,
	//		WM_NCHITTEST,
	//		wParam,
	//		lParam );
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
	case WM_ERASEBKGND:
	{
		// called before WM_PAINT to update the background of the window client region
		// pass it on to DefWindowProc(), which uses the WNDCLASSEX::hbrBackground you selected
		break;
	}
	case WM_PAINT:
	{
#	if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::instance();
		using namespace std::string_literals;
		console.log( "Painting without DirectX\n"s );
#	endif
		PAINTSTRUCT ps;
		HDC dc = BeginPaint( m_hWnd,
			&ps );

		LPRECT rect;
		GetClientRect( hWnd,
			&rect );
		bool bFlipFlop = true;
		HBRUSH hBrush = CreateSolidBrush( bFlipFlop ? col::Red : col::Green );
		FillRect( m_dcClientArea,
			rect,
			hBrush );

		EndPaint( hWnd,
			&ps );
		break;
	}
#endif // USE_GDIPLUS
	}//switch

	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}

void Window::setFont( const std::string &fontName )
{
	NONCLIENTMETRICS ncm{};
	ncm.cbSize = sizeof( NONCLIENTMETRICS );
	SystemParametersInfoW( SPI_GETNONCLIENTMETRICS,
		sizeof( NONCLIENTMETRICS ),
		&ncm,
		0u );
	HFONT hFont = CreateFontIndirectW( &ncm.lfMessageFont );
	hFont = CreateFontW( 13,
		0,
		0,
		0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		util::s2ws( fontName ).data() );
	SendMessageW( m_hWnd,
		WM_SETFONT,
		(WPARAM)hFont,
		MAKELPARAM( TRUE, 0 ) );
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

const std::string Window::WindowException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* Window::WindowException::what() const noexcept
{
	return KeyException::what();
}

const int Window::messageBoxPrintf( const TCHAR *caption,
	const TCHAR *format,
	... )
{
	TCHAR szBuffer[1024];
	va_list pArgs;
	va_start( pArgs,
		format );
	vswprintf_s( szBuffer,
		sizeof szBuffer / sizeof TCHAR,
		format,
		pArgs );
	va_end( pArgs );
	return MessageBoxW( m_hWnd,
		szBuffer,
		caption,
		0u );
}

#pragma warning( disable : 4312 )
void WINAPI Window::menuProc( HMENU hMenu )
{
	int nMenuItems = GetMenuItemCount( hMenu );
	unsigned flags;
	PLABELBOX pBox = ( m_hWnd == nullptr ) ? nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );

	for ( int i = 0; i < nMenuItems; ++i )
	{
		unsigned id = GetMenuItemID( hMenu,
			i );

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
				flags = ( id != IDM_EDIT_DELETE && pBox->ichSel == pBox->ichCaret ) ? MF_BYCOMMAND | MF_GRAYED :
					MF_BYCOMMAND | MF_ENABLED;
			}
			else
			{
				flags = MF_BYCOMMAND | MF_ENABLED;
			}

			EnableMenuItem( hMenu,
				id,
				flags );
			break;
		}
		case IDM_EDIT_PASTE:
		{
			if ( pBox != nullptr && pBox->fEdit )
			{
				// clipboard CF_ formats: https://learn.microsoft.com/en-us/windows/win32/dataxchg/standard-clipboard-formats
				const UINT ansiText = CF_TEXT;
				const bool bClipboardAvailable = IsClipboardFormatAvailable( ansiText ) ? MF_BYCOMMAND | MF_ENABLED :
					MF_BYCOMMAND | MF_GRAYED;

				EnableMenuItem( hMenu,
					id,
					bClipboardAvailable );
			}
			else
			{
				const bool bClipboardAvailable = IsClipboardFormatAvailable( m_clipboardFormats[0] );

				EnableMenuItem( hMenu,
					id,
					bClipboardAvailable );
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
bool Window::editCopy( UINT format /* = CF_TEXT */ )
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
	PLABELBOX pBox = ( m_hWnd == nullptr ) ? nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );

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
			HANDLE h = HeapCreate( HEAP_NO_SERIALIZE,
				( nChars + 1 ) * sizeof( TCHAR ),
				heapCanGrowInSize );
			*/
			hClipboard = GlobalAlloc( GMEM_MOVEABLE,
				( nChars + 1 ) * sizeof( TCHAR ) );
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

			memcpy( pCopiedStr,
				&pBox->atchLabel[charStartPos],
				nChars * sizeof( TCHAR ) );
			pCopiedStr[nChars] = (TCHAR) 0;	// for the null terminator

			GlobalUnlock( hClipboard );
			ASSERT_HRES_WIN32_IF_FAILED;

			// place the handle on the clipboard
			SetClipboardData( format,
				hClipboard );
			ASSERT_HRES_WIN32_IF_FAILED;
		}
		else
		{
			// if no text is selected, the label as a whole is copied
			m_pBoxLocalClip = (PLABELBOX) LocalAlloc( LMEM_FIXED,
				sizeof( LABELBOX ) );
			ASSERT_HRES_WIN32_IF_FAILED;

			if ( m_pBoxLocalClip == nullptr )
			{
				CloseClipboard();
				ASSERT_HRES_WIN32_IF_FAILED;
				return false;
			}

			memcpy( m_pBoxLocalClip,
				pBox,
				sizeof( LABELBOX ) );
			m_pBoxLocalClip->fSelected = FALSE;
			m_pBoxLocalClip->fEdit = FALSE;

			// place a registered clipboard format, the owner-display format and the CF_TEXT format on the clipboard using delayed rendering (only when and if they are needed)
			// to handle the "when they are needed" we must process the WM_RENDERFORMAT and WM_RENDERALLFORMATS messages
			SetClipboardData( CF_OWNERDISPLAY,
				nullptr );
			ASSERT_HRES_WIN32_IF_FAILED;
			SetClipboardData( format,
				nullptr );
			ASSERT_HRES_WIN32_IF_FAILED;
		}
	}

	CloseClipboard();
	ASSERT_HRES_WIN32_IF_FAILED;
	return true;
}
#pragma warning( default : 4312 )

#pragma warning( disable : 4312 )
bool Window::editPaste( UINT format /* = CF_TEXT */ )
{
	if ( !IsClipboardFormatAvailable( format ) )
	{
		return false;
	}

	// get a pointer to the structure for the selected label
	PLABELBOX pBox = ( m_hWnd == nullptr ) ? nullptr :
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
	PLABELBOX pBox = ( m_hWnd == nullptr ) ? nullptr :
		(PLABELBOX) GetWindowLongW( m_hWnd, 0 );
	ASSERT_HRES_WIN32_IF_FAILED;

	if ( format == CF_TEXT || format == CF_UNICODETEXT )
	{
		// allocate a buffer for the text
		int nChars = m_pBoxLocalClip->cchLabel;

		hClipboard = GlobalAlloc( GMEM_MOVEABLE,
			(nChars + 1) * sizeof( TCHAR ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		// copy the text from the m_pBoxLocalClip
		wchar_t *pCopiedStr = static_cast<wchar_t*>( GlobalLock( hClipboard ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		memcpy( pCopiedStr,
			m_pBoxLocalClip->atchLabel,
			nChars * sizeof( TCHAR ) );
		pCopiedStr[nChars] = (TCHAR)0;

		GlobalUnlock( hClipboard );
		ASSERT_HRES_WIN32_IF_FAILED;

		// place the handle on the clipboard
		SetClipboardData( format,
			hClipboard );
	}
	else
	{
		hClipboard = GlobalAlloc( GMEM_MOVEABLE,
			sizeof( LABELBOX ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		pBox = static_cast<PLABELBOX>( GlobalLock( hClipboard ) );
		ASSERT_HRES_WIN32_IF_FAILED;

		memcpy( pBox,
			m_pBoxLocalClip,
			sizeof( LABELBOX ) );

		GlobalUnlock( hClipboard );
		ASSERT_HRES_WIN32_IF_FAILED;

		SetClipboardData( format,
			hClipboard );
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

/*
Dialog::Dialog( const std::string &name )
	:
	m_name{name}
{
	HINSTANCE hInstance = GetModuleHandleW( nullptr );
	ASSERT( hInstance != nullptr, "HINSTANCE is null!" );

	HRESULT hres;

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof WNDCLASSEXW;
	wc.lpfnWndProc = (WNDPROC) dialogProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)( CreateSolidBrush( RGB( 128, 128, 128 ) ) );
	wc.lpszClassName = m_name.data();
	RegisterClassExW( &wc );
	ASSERT_HRES_WIN32_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::instance();
	using namespace std::string_literals;
	console.log( "Dialog Window : "s + util::ws2s( m_name ) + " created.\n"s );
#endif
}

Dialog::~Dialog() noexcept
{
	UnregisterClassW( m_name.data(),
		GetModuleHandleW( nullptr ) );
}

Dialog::Dialog( Dialog &&rhs ) noexcept
	:
	m_name{std::move( rhs.m_name )},
	m_hWnd{rhs.m_hWnd}
{

}

Dialog& Dialog::operator=( Dialog &&rhs ) noexcept
{
	Dialog tmp{std::move( rhs )};
	std::swap( *this,
		tmp );
	return *this;
}

LRESULT CALLBACK Dialog::dialogProc( HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
	switch ( uMsg )
	{
		HRESULT hres;
		case WM_CREATE:
		{
			CreateWindowW( L"BUTTON",
				L"Ok",
				WS_VISIBLE | WS_CHILD,
				50,
				50,
				80,
				25,
				hWnd,
				reinterpret_cast<HMENU>( 1 ),
				nullptr,
				nullptr );
			ASSERT_HRES_WIN32_IF_FAILED;
			return 0;
		}
		case WM_COMMAND:
		{
			DestroyWindow( hWnd );
			return 0;
		}
		case WM_CLOSE:
			DestroyWindow( hWnd );
			return 0;
	}
	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}

std::string Dialog::getName() const noexcept
{
	return m_name;
}

void Dialog::setHwnd( HWND hWnd )
{
	m_hWnd = hWnd;
}

HWND Dialog::getHwnd() const noexcept
{
	return m_hWnd;
}*/