#pragma once

#include <optional>
#include <memory>
#include <string>
#include "winner.h"
#include "key_exception.h"
#include "keyboard.h"
#include "mouse.h"
#include "splash_window.h"


class Graphics;

class Window
{
private:
	class WindowException final
		: public KeyException
	{
	public:
		WindowException( int line, const char *file, const char *function,
			const std::string &msg ) noexcept;

		virtual const std::string getType() const noexcept override final;
		virtual const char *what() const noexcept override final;
	};

	class WindowClass final
	{
		static inline WindowClass *m_pInstance;
	private:
		std::string m_name;
		ATOM m_classAtom;
	private:
		WindowClass( const std::string &name );
	public:
		~WindowClass() noexcept;
		WindowClass( const WindowClass &rhs ) = delete;
		WindowClass& operator=( const WindowClass &rhs ) = delete;
		WindowClass( WindowClass &&rhs ) noexcept;
		WindowClass& operator=( WindowClass &&rhs ) noexcept;

		static WindowClass& getInstance( const std::string &name );
		static void resetInstance();
		std::string getName() noexcept;
	};
	//class Dialog final
	//{
	//private:
	//	std::wstring m_name;
	//	HWND m_hWnd;
	//public:
	//	Dialog( const std::wstring &name );
	//	~Dialog() noexcept;
	//	Dialog( const Dialog &rhs ) = delete;
	//	Dialog& operator=( const Dialog &rhs ) = delete;
	//	Dialog( Dialog &&rhs ) noexcept;
	//	Dialog& operator=( Dialog &&rhs ) noexcept;
	//
	//	static LRESULT CALLBACK dialogProc( HWND hWnd, UINT uMsg, WPARAM wParam,
	//		LPARAM lParam );
	//	std::wstring getName() const noexcept;
	//	void setHwnd( HWND hWnd );
	//	HWND getHwnd() const noexcept;
	//};
private:
	static inline WindowClass *m_pWindowClass;
	static inline Keyboard m_keyboard;
	static inline Mouse m_mouse;
	static inline std::vector<BYTE> m_rawInputBuffer;
	//static inline HACCEL m_hAcceleratorTable;
	//static inline std::unique_ptr<SplashWindow> m_splash;
	bool m_bCursorEnabled = true;
	int m_width;
	int m_height;
	std::string m_name;
	HWND m_hWnd;
	//HMENU m_hTrayIconPopupMenu;
	std::unique_ptr<Graphics> m_pGraphics;
	HDC m_dc;
	WINDOWINFO m_info;
	//NOTIFYICONDATA m_notifyIconData;
	//std::unique_ptr<Dialog> m_pDialogAbout;

	static LRESULT CALLBACK windowProc( HWND pWndHandle, unsigned uMsg,
		WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK windowProcDelegate( HWND pWndHandle, unsigned uMsg,
		WPARAM wParam, LPARAM lParam );
public:
	Window();
	Window( int width, int height, const char *name );
	~Window();
	Window( const Window &rhs ) = delete;
	Window& operator=( const Window &rhs ) = delete;
	Window( Window &&rhs ) noexcept;
	Window& operator=( Window &&rhs ) noexcept;
	
	static Keyboard& getKeyboard() noexcept;
	static Mouse& getMouse() noexcept;
	static WindowClass& getWindowClass() noexcept;
	static bool isDescendantOf( HWND parent, HWND hWnd ) noexcept;
	std::optional<int> messageLoop() noexcept;
	void setEnable( bool b );
	void setOnTop();
	HWND setFocus();
	void goFullscreen() noexcept;
	void goWindowed() noexcept;
	void setBorderless() const noexcept;
	void setBorderfull() const noexcept;
	HWND getParent() const noexcept;
	void setTitle( const std::wstring &title );
	std::string getTitle() const noexcept;
	void enableCursor() noexcept;
	void disableCursor() noexcept;
	bool isCursorEnabled() const noexcept;
	void displayMessageBox( const std::wstring &title, const std::wstring &message ) const;
	bool isActive() const noexcept;
	std::string getName() const noexcept;
	void minimize();
	void restore();
	//void setupNotifyIconData();
	bool isMinimized() const noexcept;
	bool isMaximized() const noexcept;
	operator bool() const noexcept;
	Graphics& getGraphics();
	HWND getHandle() const noexcept;
	HDC getDc() const noexcept;
	WINDOWINFO getInfo() const noexcept;
	int messageBoxPrintf( const TCHAR *caption, const TCHAR *format, ... );
private:
	void configureDc();
	void confineCursor() noexcept;
	void freeCursor() noexcept;
	void displayCursor() noexcept;
	void hideCursor() noexcept;
	void enableImGuiMouse() noexcept;
	void disableImGuiMouse() noexcept;
	LRESULT windowProc_impl( _In_ HWND pWndHandle, _In_ unsigned msg,
		_In_ WPARAM wparam, _In_ LPARAM lparam );
	LRESULT windowProc_impl2d( _In_ HWND pWndHandle, _In_ unsigned msg,
		_In_ WPARAM wparam, _In_ LPARAM lparam );
	void setFont( const std::wstring &fontName );
};


#define throwWindowException( msg ) throw WindowException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );