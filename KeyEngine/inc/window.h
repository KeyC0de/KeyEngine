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

 
class Graphics;

class Window
	: public NonCopyable
{
	class WindowException final
		: public KeyException
	{
	public:
		WindowException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		virtual const std::string getType() const noexcept override final;
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
	static inline Keyboard m_keyboard;
	static inline Mouse m_mouse;
	static inline std::vector<BYTE> m_rawInputBuffer;
	static inline HACCEL m_hAcceleratorTable;
	static inline PLABELBOX m_pBoxLocalClip;
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
	//std::unique_ptr<SplashWindow> m_splash;
private:
	static LRESULT CALLBACK windowProcDelegate( _In_ const HWND pWndHandle, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
public:
	static LRESULT CALLBACK windowProc( _In_ const HWND pWnd, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
	static LRESULT CALLBACK dialogProc( _In_ const HWND hWnd, _In_ const unsigned uMsg, _In_ const WPARAM wParam, _In_ const LPARAM lParam );
	static Keyboard& keyboard() noexcept;
	static Mouse& mouse() noexcept;
	static bool isDescendantOf( const HWND targethWnd, const HWND parent ) noexcept;
	static void saveClipboardTextAsVar();
public:
	Window( const int width, const int height, const char *name, const char *className, const WNDPROC windowProcedure, const int x, const int y, const ColorBGRA bgColor = {255, 255, 255}, const HMENU hMenu = nullptr, const Window *parent = nullptr );
	~Window();
	Window( Window &&rhs ) noexcept;
	Window& operator=( Window &&rhs ) noexcept;

	std::optional<int> messageLoop() noexcept;
	void setEnable( const bool b );
	void setOnTop();
	const HWND setFocus();
	//===================================================
	//	\function	hasParent
	//	\brief  returns true if this Window has a parent that is not the Desktop window
	//	\date	2022/09/22 19:52
	const bool hasParent() const noexcept;
	const HWND getParent() const noexcept;
	void setTitle( const std::string &title );
	const std::string& getTitle() const noexcept;
	void enableCursor() noexcept;
	void disableCursor() noexcept;
	bool isCursorEnabled() const noexcept;
	void displayMessageBox( const std::string &title, const std::string &message ) const;
	bool isActive() const noexcept;
	const std::string& getName() const noexcept;
	void minimize();
	void restore();
	bool isMinimized() const noexcept;
	bool isMaximized() const noexcept;
	operator bool() const noexcept;
	Graphics& getGraphics();
	const HWND getHandle() const noexcept;
	//===================================================
	//	\function	getDc
	//	\brief  get Window's Device Context (DC)
	//	\date	2022/09/22 19:35
	HDC getDc() const noexcept;
	WINDOWINFO getInfo() const noexcept;
	const int messageBoxPrintf( const TCHAR *caption, const TCHAR *format, ... );
	unsigned getWindowStyles() const noexcept;
	unsigned getWindowStylesEx() const noexcept;
	void setWindowStyles( const unsigned windowStyles ) const noexcept;
	void setWindowStylesEx( const unsigned windowStylesEx ) const noexcept;
	void setBorderless() const noexcept;
	void setRedrawing( const bool bRedraw );
	void showMenu( const HMENU hMenu );
	void hideMenu();
	const HMENU getTopMenu() const noexcept;
	HWND getDesktop() const noexcept;
	int calcX() const noexcept;
	int calcY() const noexcept;
	int calcWidth() const noexcept;
	int calcHeight() const noexcept;
	HWND getConsoleHandle() const;
	const WindowClass& getWindowClass() noexcept;
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
	bool editCopy( unsigned format = CLIPBOARD_TEXT_FORMAT );
	bool editPaste( unsigned format = CLIPBOARD_TEXT_FORMAT );
	void renderClipboardFormat( unsigned format = CLIPBOARD_TEXT_FORMAT );
	//===================================================
	//	\function	editDelete
	//	\brief  delete selected text, eg as if you pressed "cut" or the "delete" key
	//	\date	2022/09/18 18:01
	void editDelete();
	//===================================================
	//	\function	destroyMenu
	//	\brief  The destruction of a menu is done automatically when the menu is assigned to a window, otherwise you must call this function to free the HMENU's	resources
	//	\date	2022/09/22 19:31
	void destroyMenu( const HMENU hMenu );
	void setupTrayIcon();
	void showTrayIcon() noexcept;
	void deleteTrayIcon() noexcept;
};


#define THROW_WINDOW_EXCEPTION( msg ) throw WindowException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );