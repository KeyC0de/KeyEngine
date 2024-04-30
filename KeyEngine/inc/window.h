#pragma once

#include <optional>
#include <memory>
#include <string>
#include <array>
#include "winner.h"
#include "color.h"
#include "key_exception.h"
#include "keyboard.h"
#include "mouse.h"
#include "non_copyable.h"
#include "windows_hidden_defs.h"

#if UNICODE
#	define CLIPBOARD_TEXT_FORMAT	CF_UNICODETEXT
#else
#	define CLIPBOARD_TEXT_FORMAT	CF_TEXT
#endif

#define BUTTON_TEXT_MAX_LENGTH		80

// #WARNING! These identifiers must be unique among all window classes currently present in the OS
#define MAIN_WINDOW_CLASS_NAME				"KeyEngine_Main_Window_Class"
#define MODAL_DIALOG_CLASS_NAME				"KeyEngine_Modal_Dialog_Class"
#define MODELESS_DIALOG_CLASS_NAME			"KeyEngine_Modeless_Dialog_Class"
#define SPLASH_WINDOW_CLASS_NAME			"KeyEngine_Splash_Window_Class"


class Graphics;

class Window
	: public NonCopyable
{
	class WindowException final
		: public KeyException
	{
	public:
		WindowException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		virtual std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};

	class WindowClass final
		: public NonCopyable
	{
		std::string m_name;
		ATOM m_classAtom;
	public:
		WindowClass( const char *name, const WNDPROC windowProcedure, const ColorBGRA bgColor );
		WindowClass( WindowClass &&rhs ) noexcept;
		WindowClass& operator=( WindowClass &&rhs ) noexcept;
		~WindowClass() noexcept;

		const std::string& getName() noexcept;
	};
private:
	static inline Keyboard s_keyboard;
	static inline Mouse s_mouse;
	static inline std::vector<BYTE> s_rawInputBuffer;
	static inline HACCEL s_hAcceleratorTable;
	static inline PLABELBOX s_pBoxLocalClip;
	static inline UINT_PTR s_timerEvent = 0ui64;
private:
	WindowClass m_windowClass;
	bool m_bCursorEnabled = true;
	std::string m_name;
	std::string m_title;
	HWND m_hWnd;
	std::unique_ptr<Graphics> m_pGraphics;
	WINDOWINFO m_info;
	std::array<unsigned, 8> m_clipboardFormats;	// {0: unicode format,...}
	HMENU m_hTrayIconPopupMenu;
	NOTIFYICONDATA m_trayIconData;
	std::unique_ptr<Window> m_pModalDialog;
	std::unique_ptr<Window> m_pSplash;
private:
	static LRESULT CALLBACK windowProcDelegate( _In_ const HWND pWndHandle, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
public:
	//	\function	messageBoxPrintf	||	\date	2023/04/08 11:02
	//	\brief	a wrapper over Windows MessageBox function, first argument `window` can be nullptr
	static int messageBoxPrintf( const HWND window, const TCHAR *caption, const TCHAR *format, ... );
	static LRESULT CALLBACK windowProc( _In_ const HWND pWnd, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
	static LRESULT CALLBACK dialogProc( _In_ const HWND hWnd, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
	static LRESULT CALLBACK splashWindowProc( _In_ const HWND hWnd, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
	static void CALLBACK splashWindowTimerProc( _In_ const HWND hWnd, _In_ const unsigned uMsg, _In_ const unsigned idEvent, _In_ const DWORD time );
	static Keyboard& getKeyboard() noexcept;
	static Mouse& getMouse() noexcept;
	static bool isDescendantOf( const HWND targethWnd, const HWND parent ) noexcept;
	static void saveClipboardTextAsVar();
	//	\function	convertHiconToHbitmap	||	\date	2022/10/02 1:51
	//	\brief	hIcon vs hBitmap
	//			hIcon is a handle to one of the system resources that Windows OS requires for providing a graphical USER interface.
	//			SHELL applications like Windows Explorer, Desktop etc, use this system resource extensively.
	//			hBITMAP is a graphical object which can be manipulated with GDI. hBitmaps can be altered with GDI either by selecting it to a DC or by manipulating the bits directly. GDI cannot handle an hICON.
	static HBITMAP convertHiconToHbitmap( HICON hIcon );
	static HICON convertHbitmapToHicon( HBITMAP bitmap );
public:
	Window( const int width, const int height, const char *name, const char *className, const WNDPROC windowProcedure, const int x, const int y, const ColorBGRA bgColor = {255, 255, 255}, const HMENU hMenu = nullptr, const Window *parent = nullptr );
	Window( Window &&rhs ) noexcept;
	Window& operator=( Window &&rhs ) noexcept;
	~Window();

	std::optional<int> messageLoop() noexcept;
	void setEnable( const bool b );
	void setOnTop();
	const HWND setFocus();
	//	\function	hasParent	||	\date	2022/09/22 19:52
	//	\brief	returns true if this Window has a parent that is not the Desktop window
	bool hasParent() const noexcept;
	const HWND getParent() const noexcept;
	void setTitle( const std::string &title );
	const std::string& getTitle() const noexcept;
	void enableCursor() noexcept;
	void disableCursor() noexcept;
	bool isCursorEnabled() const noexcept;
	bool displayMessageBoxYesNo( const std::string &title, const std::string &message ) const;
	bool isActive() const noexcept;
	const std::string& getName() const noexcept;
	void minimize();
	void restore();
	bool isHidden() const noexcept;	// isVisible(), or isMinimized()
	bool isMinimized() const noexcept;
	bool isMaximized() const noexcept;
	operator bool() const noexcept;
	Graphics& getGraphics();
	const HWND getHandle() const noexcept;
	//	\function	getDc	||	\date	2022/09/22 19:35
	//	\brief	get Window's Device Context (DC) - a "device context" contains information about the drawing attributes of a device such as a display or a printer, so if you have two different DC's, you're drawing in two different places; like a file handle for drawing
	HDC getDc() const noexcept;
	WINDOWINFO getInfo() const noexcept;
	unsigned getWindowStyles() const noexcept;
	unsigned getWindowStylesEx() const noexcept;
	void setWindowStyles( const unsigned windowStyles ) const noexcept;
	void setWindowStylesEx( const unsigned windowStylesEx ) const noexcept;
	void setBorderless() const noexcept;
	//	\function	setRedrawing	||	\date	2022/10/29 12:18
	//	\brief	prevent intermediate redrawing
	void setRedrawing( const bool bRedraw );
	void showMenu( const HMENU hMenu );
	void hideMenu();
	const HMENU getTopMenu() const noexcept;
	HWND getDesktop() const noexcept;
	int getX() const noexcept;
	int getY() const noexcept;
	int getWidth() const noexcept;
	int getHeight() const noexcept;
	HWND getConsoleHandle() const;
	const WindowClass& getWindowClass() noexcept;
	//	\function	setupSplashWindow	||	\date	2022/10/01 20:30
	//	\brief	create an Alpha enabled Splash window for the main application window
	//			We'll use a layered window to do it (recommended method for Windows 2000+).
	//			The beauty of layered windows and the UpdateLayeredWindow function is that the splash window doesn't have to respond to WM_PAINT messages; Windows will paint it (and blend it correctly with the windows below it) by default.
	//			#WARNING: load the BMP with black color for the transparent places
	void setupSplashWindow( HBITMAP hSplashBitmap );
private:
	void confineCursor() noexcept;
	void freeCursor() noexcept;
	void showCursor() noexcept;
	void hideCursor() noexcept;
	void enableImGuiMouse() noexcept;
	void disableImGuiMouse() noexcept;
	LRESULT windowProc_impl3d( _In_ const HWND pWndHandle, _In_ const unsigned msg, _In_ const WPARAM wparam, _In_ const LPARAM lparam );
	LRESULT windowProc_impl2d( _In_ const HWND pWndHandle, _In_ const unsigned msg, _In_ const WPARAM wparam, _In_ const LPARAM lparam );
	void setFont( const std::string &fontName );
	void resize( const int width, const int height, const unsigned flags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE ) const noexcept;
	// Menu related functions
	void WINAPI processMenu( HMENU hMenu );
	bool editCopy( const unsigned format = CLIPBOARD_TEXT_FORMAT );
	bool editPaste( const unsigned format = CLIPBOARD_TEXT_FORMAT );
	void renderClipboardFormat( unsigned format = CLIPBOARD_TEXT_FORMAT );
	//	\function	editDelete	||	\date	2022/09/18 18:01
	//	\brief	delete selected text, eg as if you pressed "cut" or the "delete" key
	void editDelete();
	void setupTrayIcon();
	void showTrayIcon() noexcept;
	void deleteTrayIcon() noexcept;
};


#define THROW_WINDOW_EXCEPTION( msg ) throw WindowException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );