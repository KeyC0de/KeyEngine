#pragma once

#include "winner.h"


class SplashWindow final
{
	bool m_bVisible;
	HWND m_hWnd;
public:
	SplashWindow( HWND hWndParent, HINSTANCE hInst, int resourceId,
		std::pair<int,int> dims );
	~SplashWindow();

	void display();
	void hide();
	bool isVisible() const noexcept;
	void messageLoop();
private:
	static LRESULT CALLBACK windowProc( HWND pWndHandle, unsigned uMsg,
		WPARAM wParam, LPARAM lParam );
//	static LRESULT CALLBACK windowProcDelegate( HWND pWndHandle, unsigned uMsg,
//		WPARAM wParam, LPARAM lParam );
//private:
//	LRESULT windowProc_impl( _In_ HWND pWndHandle, _In_ unsigned msg,
//		_In_ WPARAM wparam, _In_ LPARAM lparam );
};