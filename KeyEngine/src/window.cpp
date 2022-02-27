#include <sstream>
#include <string>
#include <windowsx.h>
#include "window.h"
#include "graphics.h"
#include "../resource.h"
#include "imgui_impl_win32.h"
#include "utils.h"
#include "os_utils.h"
#include "math_utils.h"
#include "console.h"
#include "assertions_console.h"
#include "graphics_mode.h"

//#define WM_TRAY_ICON			10001
//#define ID_TRAY_APP_ICON		20001
//#define ID_TRAY_ICON_EXIT		20002


Window::WindowClass::WindowClass( const std::string& name )
	:
	m_name{name}
{
	HINSTANCE hInstance = THIS_INSTANCE;
	ASSERT( hInstance != nullptr, "HINSTANCE is null!" );

	HRESULT hres;

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
		MAKEINTRESOURCEW( IDC_POINTER_APP ) );
	ASSERT_HRES_WIN32_IF_FAILED( hres );
	wc.hIcon = static_cast<HICON>( LoadImageW( hInstance,
		MAKEINTRESOURCEW( IDI_ICON_APP ),
		IMAGE_ICON,
		64,
		43,
		0u ) );
	ASSERT_HRES_WIN32_IF_FAILED( hres );
	wc.hIconSm = static_cast<HICON>( LoadImageW( hInstance,
		MAKEINTRESOURCEW( IDI_ICON_APP_SMALL ),
		IMAGE_ICON,
		32,
		22,
		0u ) );
	ASSERT_HRES_WIN32_IF_FAILED( hres );
	wc.hbrBackground = CreateSolidBrush( RGB( 255, 255, 255 ) );
	wc.lpszMenuName = nullptr;
	const std::wstring className = util::s2ws( name );
	wc.lpszClassName = className.data();
	m_classAtom = RegisterClassExW( &wc );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

#if defined _DEBUG && !defined NDEBUG
	auto& console = KeyConsole::getInstance();
	using namespace std::string_literals;
	console.log( "\n\nApplication Instance: "s
		+ math::toHexString( hInstance )
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

Window::WindowClass::WindowClass( WindowClass&& rhs ) noexcept
	:
	m_name{std::move( rhs.m_name )},
	m_classAtom{rhs.m_classAtom}
{
	m_pInstance = rhs.m_pInstance;
}

Window::WindowClass& Window::WindowClass::operator=( WindowClass&& rhs ) noexcept
{
	m_pInstance = rhs.m_pInstance;
	m_name = std::move( rhs.m_name );
	m_classAtom = rhs.m_classAtom;
	return *this;
}

Window::WindowClass& Window::WindowClass::getInstance( const std::string& name )
{
	if ( m_pInstance == nullptr )
	{
		m_pInstance = new WindowClass{name};
	}
	return *m_pInstance;
}

void Window::WindowClass::resetInstance()
{
	if ( m_pInstance != nullptr )
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

std::string Window::WindowClass::getName() noexcept
{
	return m_name;
}

/*
Window::Dialog::Dialog( const std::wstring& name )
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
	ASSERT_HRES_WIN32_IF_FAILED( hres );

#if defined _DEBUG && !defined NDEBUG
	auto& console = KeyConsole::getInstance();
	using namespace std::string_literals;
	console.log( "Dialog Window : "s
		+ util::ws2s( m_name )
		+ " created.\n"s );
#endif
}

Window::Dialog::~Dialog() noexcept
{
	UnregisterClassW( m_name.data(),
		GetModuleHandleW( nullptr ) );
}

Window::Dialog::Dialog( Dialog&& rhs ) noexcept
	:
	m_name{std::move( rhs.m_name )},
	m_hWnd{rhs.m_hWnd}
{

}

Window::Dialog& Window::Dialog::operator=( Dialog&& rhs ) noexcept
{
	std::swap( m_name, rhs.m_name );
	m_hWnd = rhs.m_hWnd;
	return *this;
}

LRESULT CALLBACK Window::Dialog::dialogProc( HWND hWnd,
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
			ASSERT_HRES_WIN32_IF_FAILED( hres );
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

std::wstring Window::Dialog::getName() const noexcept
{
	return m_name;
}

void Window::Dialog::setHwnd( HWND hWnd )
{
	m_hWnd = hWnd;
}

HWND Window::Dialog::getHwnd() const noexcept
{
	return m_hWnd;
}
*/

Window::Window()
	:
	Window(640, 480, "Default Window Title")
{

}

Window::Window( int width,
	int height,
	const char* name )
	:
	m_width(width),
	m_height(height),
	m_name{name}
{
	HRESULT hres;

	m_pWindowClass = &WindowClass::getInstance( "KeyEngine_Window_Class" );

	RECT rect{0, 0, width, height};
	BOOL ret = AdjustWindowRect( &rect,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		FALSE );
	if ( !ret )
	{
		throwWindowException( "Failed to adjust window rectangle" );
	}

	uint32_t windowExStyles = WS_EX_OVERLAPPEDWINDOW;
	uint32_t windowStyles = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU
		| WS_OVERLAPPEDWINDOW;
	const std::wstring className = util::s2ws( m_pWindowClass->getName() );
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
	ASSERT_HRES_WIN32_IF_FAILED( hres );
	if ( !m_hWnd )
	{
		throwWindowException( "Failed to create Window" );
	}

	// mouse raw input
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01u;	// mouse page
	rid.usUsage = 0x02u;		// mouse usage
	rid.dwFlags = 0u;
	rid.hwndTarget = nullptr;
	ret = RegisterRawInputDevices( &rid,
		1u,
		sizeof RAWINPUTDEVICE );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

	m_info.cbSize = sizeof WINDOWINFO;
	GetWindowInfo( m_hWnd,
		&m_info );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

	// Accelerators
	//m_hAcceleratorTable = LoadAcceleratorsW( hInstance,
	//	MAKEINTRESOURCEW( ID_ACCEL_TABLE_APP ) );
	//ASSERT_HRES_WIN32_IF_FAILED( hres );

	// Tray icon setup
	//setupNotifyIconData();

	// Initialize ImGui
	if constexpr( GraphicsMode::get() == GraphicsMode::_3D )
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
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		ImGui_ImplWin32_Shutdown();
	}
	DestroyWindow( m_hWnd );
}

Window::Window( Window&& rhs ) noexcept
	:
	m_bCursorEnabled{std::move( rhs.m_bCursorEnabled )},
	m_width{rhs.m_width},
	m_height{rhs.m_height},
	m_name{rhs.m_name},
	m_hWnd{rhs.m_hWnd},
	m_pGraphics{std::move( rhs.m_pGraphics )},
	m_dc{rhs.m_dc},
	m_info{std::move( rhs.m_info )}
{
	m_pWindowClass = std::move( rhs.m_pWindowClass );
	m_keyboard = std::move( rhs.m_keyboard );
	rhs.m_keyboard = {};
	m_mouse = std::move( rhs.m_mouse );
	rhs.m_mouse = {};
	m_rawInputBuffer = std::move( rhs.m_rawInputBuffer );
	rhs.m_rawInputBuffer = {};
}

Window& Window::operator=( Window&& rhs ) noexcept
{
	std::swap( m_bCursorEnabled, rhs.m_bCursorEnabled );
	m_width = rhs.m_width;
	m_height = rhs.m_height;
	std::swap( m_name, rhs.m_name );
	m_hWnd = rhs.m_hWnd;
	std::swap( m_pGraphics, rhs.m_pGraphics );
	m_dc = rhs.m_dc;
	std::swap( m_info, rhs.m_info );
	m_pWindowClass = std::move( rhs.m_pWindowClass );
	m_keyboard = std::move( rhs.m_keyboard );
	rhs.m_keyboard = {};
	m_mouse = std::move( rhs.m_mouse );
	rhs.m_mouse = {};
	m_rawInputBuffer = std::move( rhs.m_rawInputBuffer );
	rhs.m_rawInputBuffer = {};
	return *this;
}

void Window::setEnable( bool b )
{
	EnableWindow( m_hWnd,
		b ?
			TRUE :
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

HWND Window::setFocus()
{
	HWND previouslyFocusedWindow = SetFocus( m_hWnd );
	return previouslyFocusedWindow;
}

void Window::goFullscreen() noexcept
{
	/*
	If the application chooses to go to full-screen mode by itself, then it can call IDXGISwapChain::SetFullscreenState and pass an explicit IDXGIOutput1 (or NULL, if the application is happy to let DXGI decide).
	*/
}

void Window::goWindowed() noexcept
{

}

void Window::setBorderless() const noexcept
{
	::SetWindowLongPtrW( m_hWnd,
		GWL_STYLE,
		::GetWindowLongPtrW( m_hWnd, GWL_STYLE )
			& ~( WS_BORDER | WS_DLGFRAME | WS_THICKFRAME ) );
	::SetWindowLongPtrW( m_hWnd,
		GWL_EXSTYLE,
		::GetWindowLongPtrW( m_hWnd, GWL_EXSTYLE ) & ~WS_EX_DLGMODALFRAME );
}

void Window::setBorderfull() const noexcept
{

}

HWND Window::getParent() const noexcept
{
	return GetParent( m_hWnd );
}

void Window::setTitle( const std::wstring& title )
{
	SetWindowTextW( m_hWnd,
		title.data() );
	HRESULT hres;
	ASSERT_HRES_WIN32_IF_FAILED( hres );
}

std::string Window::getTitle() const noexcept
{
	static const short maxTitleCharSize = 128;
	wchar_t title[maxTitleCharSize];
	GetWindowTextW( m_hWnd,
		title,
		maxTitleCharSize );
	return util::ws2s( std::wstring{title} );
}

void Window::enableCursor() noexcept
{
	m_bCursorEnabled = true;
	displayCursor();
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		enableImGuiMouse();
	}
	freeCursor();
}

void Window::disableCursor() noexcept
{
	m_bCursorEnabled = false;
	hideCursor();
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
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

void Window::displayMessageBox( const std::wstring& title,
	const std::wstring& message ) const
{
	MessageBoxW( m_hWnd,
		message.c_str(),
		title.c_str(),
		MB_OKCANCEL );
}

bool Window::isActive() const noexcept
{
	return GetActiveWindow() == m_hWnd;
}

std::string Window::getName() const noexcept
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
		//if ( !TranslateAcceleratorW( m_hWnd, m_hAcceleratorTable, &msg ) )
		//{
			TranslateMessage( &msg );
			DispatchMessageW( &msg );
		//}
	}
	return std::nullopt;
}

Graphics& Window::getGraphics()
{
	return *m_pGraphics;
}

HWND Window::getHandle() const noexcept
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

Keyboard& Window::getKeyboard() noexcept
{
	return m_keyboard;
}

Mouse& Window::getMouse() noexcept
{
	return m_mouse;
}

Window::WindowClass& Window::getWindowClass() noexcept
{
	return *m_pWindowClass;
}

bool Window::isDescendantOf( HWND parent,
	HWND hWnd ) noexcept
{
	return IsChild( parent, hWnd ) == 0 ?
		false :
		true;
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

void Window::displayCursor() noexcept
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

LRESULT CALLBACK Window::windowProc( HWND hWnd,
	unsigned uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto& console = KeyConsole::getInstance();
	console.log( "windowProc() entered\n" );
#endif

	if ( uMsg == WM_NCCREATE )
	{
#if defined _DEBUG && !defined NDEBUG
		console.log( "windowProc() WM_NCCREATE message\n" );
#endif
		const CREATESTRUCTW* const cs = (CREATESTRUCTW*) lParam;
		Window* this_wnd = reinterpret_cast<Window*>( cs->lpCreateParams );

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

LRESULT CALLBACK Window::windowProcDelegate( HWND hWnd,
	unsigned uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
	Window* this_wnd = reinterpret_cast<Window*>( GetWindowLongPtrW( hWnd,
		GWLP_USERDATA ) );
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		return this_wnd->windowProc_impl( hWnd,
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

LRESULT Window::windowProc_impl2d( _In_ HWND hWnd,
	_In_ unsigned uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam )
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
		auto& console = KeyConsole::getInstance();
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
				displayCursor();
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
		unsigned size;
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
		auto& ri = reinterpret_cast<const RAWINPUT&>( *m_rawInputBuffer.data() );
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

LRESULT Window::windowProc_impl( _In_ HWND hWnd,
	_In_ unsigned uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam )
{
	// ImGui windowProc messages
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) )
	{
		return true;
	}
	const auto& imGuiIoContext = ImGui::GetIO();

	// custom messages (create with RegisterWindowMessage)

	// Predefined Windows messages
	switch( uMsg )
	{
	case WM_CREATE:
	{
//		HRESULT hres;
//		// Initialization. Controls - child windows, set default values for controls
//#if defined _DEBUG && !defined NDEBUG
//		auto& console = KeyConsole::getInstance();
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
//		ASSERT_HRES_WIN32_IF_FAILED( hres );
//		return 0;
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
		auto& console = KeyConsole::getInstance();
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
				displayCursor();
			}
		}
//		Shell_NotifyIconW( NIM_ADD,
//			&m_notifyIconData );
		break;
	}
	case WM_ENABLE:
	{
#if defined _DEBUG && !defined NDEBUG
		auto& console = KeyConsole::getInstance();
		if ( wParam == TRUE )
		{
			console.log( "Window "
				+ getTitle()
				+ " has been enabled.\n" );
		}
		else
		{
			console.log( "Window "
				+ getTitle()
				+ " has been disabled.\n" );
		}
#endif
		break;
	}
#if defined _DEBUG && !defined NDEBUG
	case WM_SHOWWINDOW:
	{
		auto& console = KeyConsole::getInstance();
		if ( wParam == TRUE )
		{
			console.log( "Window "
				+ getTitle()
				+ " is visible.\n" );
		}
		else
		{
			console.log( "Window "
				+ getTitle()
				+ " is hidden.\n" );
		}
		return 0;
	}
#endif
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
		auto& console = KeyConsole::getInstance();
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
/*	case WM_SIZE:
	{
		if (g_pSwapChain)
		{
			g_pd3dDeviceContext->OMSetRenderTargets(0, 0, 0);

			// Release all outstanding references to the swap chain's buffers.
			g_pRenderTargetView->Release();

			HRESULT hr;
			// Preserve the existing buffer count and format.
			// Automatically choose the width and height to match the client rect for HWNDs.
			hr = g_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			
			// Perform error handling here!

			// Get buffer and create a render-target-view.
			ID3D11Texture2D* pBuffer;
			hr = g_pSwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D),
										 (void**) &pBuffer );
			// Perform error handling here!

			hr = g_pd3dDevice->CreateRenderTargetView(pBuffer, NULL,
													 &g_pRenderTargetView);
			// Perform error handling here!
			pBuffer->Release();

			g_pd3dDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL );

			// Set up the viewport.
			D3D11_VIEWPORT vp;
			vp.Width = width;
			vp.Height = height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			g_pd3dDeviceContext->RSSetViewports( 1, &vp );
		}
		return 1;

Prevent window resizing: Instead resize by ingame option - choosing resolution:
	To resize the output while either full screen or windowed, we recommend to call IDXGISwapChain::ResizeTarget, since this method resizes the target window also. Since the target window is resized, the operating system sends WM_SIZE, and your code will naturally call IDXGISwapChain::ResizeBuffers in response. It's thus a waste of effort to resize your buffers, and then subsequently resize the target.
	}*/
	/// Menu messages
	case WM_COMMAND:
	{// sent when the user selects a command item from a menu, when a control sends a notification message to its parent window, or when an accelerator keystroke is translated
//		UINT id = LOWORD( wParam );
//		UINT bAccelerator = HIWORD( wParam ); // lParam is not used
//		switch ( id )
//		{
//		case ID_HELP_ABOUT:
//		{
//			HRESULT hres;
//			m_pDialogAbout.reset();
//			m_pDialogAbout = std::make_unique<Dialog>( L"KeyEngine_Dialog_Class" );
//			HWND hWndDialogAbout = CreateWindowExW( WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
//				L"KeyEngine_Dialog_Class",
//				L"Dialog Box",
//				WS_VISIBLE | WS_SYSMENU | WS_CAPTION ,
//				100,
//				100,
//				200,
//				150,
//				nullptr,
//				nullptr,
//				THIS_INSTANCE,
//				nullptr );
//			ASSERT_HRES_WIN32_IF_FAILED( hres );
//			m_pDialogAbout->setHwnd( hWndDialogAbout );
//			return 0;
//		}
//		case ID_FILE_QUIT:
//			PostQuitMessage( 0 );
//			return 0;
//		default:
//			break;
//		}
		break;
	}
	case WM_SYSCOMMAND:
	{// received when the user chooses a command from the Window menu (formerly known as
		// the system or control menu) or when the user chooses the maximize/minimize/close buttons
		// in WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter are used internally by the system
//		switch( wParam & 0xFFF0 )
//		{
//		case SC_MINIMIZE:
//			minimize();
//			return 0;
//		case SC_MAXIMIZE:
//			restore();
//			return 0;
//		}
		break;
	}
//	case WM_TRAY_ICON:
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
	/// Clipboard messages
	case WM_INITMENUPOPUP:
	{
#if defined _DEBUG && !defined NDEBUG
		auto& console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Clipboard processing\n"s );
#endif
		/*
		// clipboard processing
		// https://docs.microsoft.com/en-us/windows/win32/dataxchg/using-the-clipboard
		SetTimer(hwnd, TIMER_ID, 1000, NULL);
		static HWND hwndNextViewer;
	HGLOBAL hGlobal;
	HDC hdc;
	PTSTR pGlobal;
	PAINTSTRUCT ps;
	RECT rect;
	switch (message)
		{
	case WM_CREATE:
		hwndNextViewer = SetClipboardViewer(hwnd);
		return 0;
	case WM_CHANGECBCHAIN:
		if ((HWND)wParam == hwndNextViewer)
			hwndNextViewer = (HWND)lParam;
		else if (hwndNextViewer)
			SendMessage(hwndNextViewer, message, wParam, lParam);
		return 0;
	case WM_DRAWCLIPBOARD:
		if (hwndNextViewer)
			SendMessage(hwndNextViewer, message, wParam, lParam);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		OpenClipboard(hwnd);
#ifdef UNICODE
		hGlobal = GetClipboardData(CF_UNICODETEXT);
#else
		hGlobal = GetClipboardData(CF_TEXT);
#endif
		if (hGlobal != NULL)
		{
			pGlobal = (PTSTR)GlobalLock(hGlobal);
			DrawText(hdc, pGlobal, -1, &rect, DT_EXPANDTABS);
			GlobalUnlock(hGlobal);
		}
		CloseClipboard();
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		ChangeClipboardChain(hwnd, hwndNextViewer);
		PostQuitMessage(0);
		return 0;
	}
	*/
		break;
	}
	/// Keyboard Messages
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
		unsigned size;
		// first get the size of the input data
		if ( GetRawInputData( reinterpret_cast<HRAWINPUT>( lParam ),
			RID_INPUT,
			nullptr,
			&size,
			sizeof( RAWINPUTHEADER ) ) == -1)
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
		auto& ri = reinterpret_cast<const RAWINPUT&>( *m_rawInputBuffer.data() );
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

void Window::setFont( const std::wstring& fontName )
{
	NONCLIENTMETRICS ncm;
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
		fontName.data() );
	SendMessageW( m_hWnd,
		WM_SETFONT,
		(WPARAM)hFont,
		MAKELPARAM( TRUE, 0 ) );
}


// WindowException
Window::WindowException::WindowException( int line,
	const char* file,
	const char* function,
	const std::string& msg ) noexcept
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

int Window::messageBoxPrintf( const TCHAR* caption,
	const TCHAR* format,
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