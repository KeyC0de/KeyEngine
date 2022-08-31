#pragma once

#include "winner.h"


class SplashWindow final
{
	bool m_bVisible;
	HWND m_hWnd;
public:
	SplashWindow( HWND hWndParent, HINSTANCE hInst, const int resourceId, const std::pair<int, int> dims );
	~SplashWindow();

	void display();
	void hide();
	bool isVisible() const noexcept;
	void messageLoop();
private:
	static LRESULT CALLBACK windowProc( const HWND pWndHandle, const unsigned uMsg, const WPARAM wParam, const LPARAM lParam );
//	static LRESULT CALLBACK windowProcDelegate( const HWND pWndHandle, const unsigned uMsg, const WPARAM wParam, const LPARAM lParam );
//private:
//	LRESULT windowProc_impl( _In_ const HWND pWndHandle, _In_ const unsigned msg, _In_ const WPARAM wparam, _In_ const LPARAM lparam );
};