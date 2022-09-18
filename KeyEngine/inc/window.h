#pragma once

#include <optional>
#include <memory>
#include <string>
#include <array>
#include "winner.h"
#include "key_exception.h"
#include "keyboard.h"
#include "mouse.h"
#include "splash_window.h"
#include "non_copyable.h"
#include "windows_hidden_defs.h"

 
class Graphics;

class Window
	: public NonCopyable
{
private:
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
		static inline WindowClass *m_pInstance;
	private:
		std::string m_name;
		ATOM m_classAtom;
	private:
		WindowClass( const std::string &name );
	public:
		~WindowClass() noexcept;
		WindowClass( WindowClass &&rhs ) noexcept;
		WindowClass& operator=( WindowClass &&rhs ) noexcept;

		static WindowClass& instance( const std::string &name );
		const std::string& getName() noexcept;
	};
private:
	static inline WindowClass *m_pWindowClass;
	static inline Keyboard m_keyboard;
	static inline Mouse m_mouse;
	static inline std::vector<BYTE> m_rawInputBuffer;
	static inline HACCEL m_hAcceleratorTable;
	static inline PLABELBOX m_pBoxLocalClip;
private:
	bool m_bCursorEnabled = true;
	int m_width;
	int m_height;
	std::string m_name;
	std::string m_title;
	HWND m_hWnd;
	std::unique_ptr<Graphics> m_pGraphics;
	HDC m_dc;
	WINDOWINFO m_info;
	std::array<unsigned, 8> m_clipboardFormats;	// {0: unicode format,...}
	//std::unique_ptr<SplashWindow> m_splash;
	//HMENU m_hTrayIconPopupMenu;
	//NOTIFYICONDATA m_notifyIconData;
	//std::unique_ptr<Dialog> m_pDialogAbout;
private:
	static LRESULT CALLBACK windowProc( const HWND pWndHandle, const unsigned uMsg, const WPARAM wParam, const LPARAM lParam );
	static LRESULT CALLBACK windowProcDelegate( const HWND pWndHandle, const unsigned uMsg, const WPARAM wParam, const LPARAM lParam );
public:
	static Keyboard& keyboard() noexcept;
	static Mouse& mouse() noexcept;
	static const WindowClass& getWindowClass() noexcept;
	static bool isDescendantOf( HWND targethWnd, const HWND parent ) noexcept;
	static void saveClipboardTextAsVar();
public:
	Window( const int width, const int height, const char *name );
	~Window();
	Window( Window &&rhs ) noexcept;
	Window& operator=( Window &&rhs ) noexcept;

	std::optional<int> messageLoop() noexcept;
	void setEnable( const bool b );
	void setOnTop();
	const HWND setFocus();
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
	HDC getDc() const noexcept;
	WINDOWINFO getInfo() const noexcept;
	const int messageBoxPrintf( const TCHAR *caption, const TCHAR *format, ... );
private:
	// Menu related functions
	void WINAPI menuProc( HMENU hMenu );
#ifdef UNICODE
	bool editCopy( unsigned format = CF_UNICODETEXT );
	bool editPaste( unsigned format = CF_UNICODETEXT );
	void renderClipboardFormat( unsigned format = CF_UNICODETEXT );
#else
	bool editCopy( unsigned format = CF_TEXT );
	bool editPaste( unsigned format = CF_TEXT );
	void renderClipboardFormat( unsigned format = CF_TEXT );
#endif
	//===================================================
	//	\function	editDelete
	//	\brief  delete selected text, eg as if you pressed "cut" or the "delete" key
	//	\date	2022/09/18 18:01
	void editDelete();
	//void setupNotifyIconData();
private:
	void configureDc();
	void confineCursor() noexcept;
	void freeCursor() noexcept;
	void showCursor() noexcept;
	void hideCursor() noexcept;
	void enableImGuiMouse() noexcept;
	void disableImGuiMouse() noexcept;
	LRESULT windowProc_impl3d( _In_ const HWND pWndHandle, _In_ const unsigned msg, _In_ const WPARAM wparam, _In_ const LPARAM lparam );
	LRESULT windowProc_impl2d( _In_ const HWND pWndHandle, _In_ const unsigned msg, _In_ const WPARAM wparam, _In_ const LPARAM lparam );
	void setFont( const std::string &fontName );
};


#define throwWindowException( msg ) throw WindowException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );


//class Dialog final
//	: public NonCopyable
//{
//private:
//	std::string m_name;
//	HWND m_hWnd;
//public:
//	Dialog( const std::string &name );
//	~Dialog() noexcept;
//	Dialog( Dialog &&rhs ) noexcept;
//	Dialog& operator=( Dialog &&rhs ) noexcept;
//
//	static LRESULT CALLBACK dialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
//	const std::string& getName() const noexcept;
//	void setHwnd( HWND hWnd );
//	HWND getHwnd() const noexcept;
//};