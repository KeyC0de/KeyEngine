#include "splash_window.h"
#include <string>
#include "sysmetrics.h"
#include "assertions_console.h"
#include "os_utils.h"
#include "bitmap.h"


SplashWindow::SplashWindow( HWND hWndParent,
	HINSTANCE hInst,
	int resourceId,
	std::pair<int,int> dims )
	:
	m_bVisible{false}
{
	HINSTANCE hInstance = THIS_INSTANCE;
	ASSERT( hInstance != nullptr,
		"HINSTANCE is null!" );

	HRESULT hres = 0;

	WNDCLASSEX wc{};
	wc.cbSize = sizeof WNDCLASSEX;
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;	// extra bytes
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)( CreateSolidBrush( RGB( 0, 0, 0 ) ) );
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = L"KeyEngine_Splash_Class";
	RegisterClassExW( &wc );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

	m_hWnd = CreateWindowExW( WS_EX_LAYERED	// layered window
		| WS_EX_TOPMOST
		| WS_EX_TOOLWINDOW	// don't appear in the taskbar or ALT+TAB list
		,
		wc.lpszClassName,
		L"",
		WS_OVERLAPPEDWINDOW | WS_POPUP,
		0,
		0,
		dims.first,
		dims.second,
		hWndParent,
		nullptr,
		hInst,
		nullptr );
	ASSERT_HRES_WIN32_IF_FAILED( hres );

	/// load png image here as a transparent bitmap
	auto hBitmap = Bitmap::loadFromFile( "splash.png" );

	//HBITMAP hBitmap = LoadBitmapW( hInst, MAKEINTRESOURCE( resourceId ) );
	// create "memory dc" or "compatible dc" - a dc special for bitmaps
	HDC hdcWindow = GetDC( nullptr );
	HDC hdc = CreateCompatibleDC( hdcWindow );
	ReleaseDC( nullptr,
		hdcWindow );
	//DeleteDC( hdcWindow );
	HBITMAP hBitmapObj = (HBITMAP)SelectObject( hdc,
		hBitmap );
	if ( !hBitmapObj )
	{
		return;
	}

	POINT startPoint{0, 0};
	SIZE size = {dims.first, dims.second};
	BLENDFUNCTION bf{0};
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 256;
	bf.AlphaFormat = AC_SRC_ALPHA;

	UpdateLayeredWindow( m_hWnd,
		nullptr,
		nullptr,
		&size,
		hdc,
		&startPoint,
		0,
		&bf,
		ULW_ALPHA );
	ASSERT_HRES_WIN32_IF_FAILED( hres );
	SelectObject( hdc,
		hBitmapObj );
	if ( !hBitmapObj )
	{
		return;
	}
	//DeleteObject( hBitmapObj );
	ShowWindow( m_hWnd,
		SW_SHOW );
}

SplashWindow::~SplashWindow()
{
	DestroyWindow( m_hWnd );
}

void SplashWindow::messageLoop()
{
	MSG msg;
	while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessageW( &msg );
	}
}

void SplashWindow::display()
{
	int	width, height;
	int	startX, startY;
	HDWP windefer{0};
	RECT rect;

	GetClientRect( m_hWnd,
		&rect );
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// center splash window on screen
	int screenWidth = GetSystemMetrics( SM_CXSCREEN );
	int screenHeight = GetSystemMetrics( SM_CYSCREEN );

	startX = ( screenWidth / 2 ) - ( width / 2 );
	startY = ( screenHeight / 2 ) - ( height / 2 );

	MoveWindow( m_hWnd,
		startX,
		startY,
		256,
		256,
		FALSE );

	ShowWindow( m_hWnd,
		SW_SHOW );
	UpdateWindow( m_hWnd );
	m_bVisible = true;
}

void SplashWindow::hide()
{
	ShowWindow( m_hWnd,
		SW_HIDE );
	m_bVisible = false;
}

bool SplashWindow::isVisible() const noexcept
{
	return m_bVisible;
}


LRESULT CALLBACK SplashWindow::windowProc( HWND hWnd,
	unsigned uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	console.log( "windowProc() entered\n" );
#endif

	/*
	if ( uMsg == WM_NCCREATE )
	{
		console.log( "windowProc() WM_NCCREATE message\n" );

		const CREATESTRUCTW *const cs = (CREATESTRUCTW*) lParam;
		SplashWindow *this_wnd = reinterpret_cast<SplashWindow*>( cs->lpCreateParams );

		this_wnd->m_hWnd = hWnd;

		SetWindowLongPtrW( hWnd,
			GWLP_USERDATA,
			(LONG_PTR) this_wnd );
		SetWindowLongPtrW( hWnd,
			GWLP_WNDPROC,
			(LONG_PTR) &SplashWindow::windowProcDelegate );
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
	*/
	//////////////////////////////////////////////////////////////////////
	switch( uMsg )
	{
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		return 0;
	}
	/*
	case WM_ENABLE:
	{
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
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
	case WM_ERASEBKGND:
	{
		// called before WM_PAINT to update the background of the window client region
		// pass it on to DefWindowProc(), which uses the WNDCLASSEX::hbrBackground you selected
		break;
	}
	*/
	/*
	case WM_PAINT:
	{
#	if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Painting without DirectX\n"s );
#	endif

		break;
	}
	*/
	}//switch

	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}
/*
LRESULT CALLBACK SplashWindow::windowProcDelegate( HWND hWnd,
	unsigned uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
	SplashWindow *this_wnd = reinterpret_cast<SplashWindow*>( GetWindowLongPtrW( hWnd,
		GWLP_USERDATA ) );
	return this_wnd->windowProc_impl( hWnd,
		uMsg,
		wParam,
		lParam );
}

LRESULT SplashWindow::windowProc_impl( _In_ HWND hWnd,
	_In_ unsigned uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		return 0;
	}*/
	/*
	case WM_ENABLE:
	{
#if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
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
	case WM_ERASEBKGND:
	{
		// called before WM_PAINT to update the background of the window client region
		// pass it on to DefWindowProc(), which uses the WNDCLASSEX::hbrBackground you selected
		break;
	}
	*/
	/*
	case WM_PAINT:
	{
#	if defined _DEBUG && !defined NDEBUG
		auto &console = KeyConsole::getInstance();
		using namespace std::string_literals;
		console.log( "Painting without DirectX\n"s );
#	endif

		break;
	}
	*/
	/*}//switch

	return DefWindowProcW( hWnd,
		uMsg,
		wParam,
		lParam );
}
*/