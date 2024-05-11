#pragma once

#include <string>
#include "winner.h"
#include "non_copyable.h"


//============================================================
//	\class	KeyConsole
//	\author	KeyC0de
//	\date	2020/07/10 15:10
//	\brief	interface to the process's console
//			singleton class
//			by default set to output mode
//			the mode is changed dynamically depending on print/log/read operation request
//=============================================================
class KeyConsole final
	: public NonCopyable
{
	static inline constexpr const char *s_currentVersion = "v0.4";
	static inline constexpr const char *s_defaultConsoleTitle = "Debug Console - ";
	static inline KeyConsole *s_pInstance;
	FILE *m_fp;
	DWORD m_stdDevice;
	FILE *m_hMode;			// set this when you print/log/read to stdout/stderr/stdin
	HANDLE m_hConsole;
	WORD m_consoleAttributesDefault;
	WORD m_consoleAttributes;

	enum LogCategory
	{
		None,
		OS,
		Util,
		Graphics,
		Animation,
		Gameplay,
		UI,
		Multiplayer,
		Error,
	};
private:
	KeyConsole( const std::string &fontName = "Lucida Console" );
public:
	static KeyConsole& getInstance() noexcept;
	//	\function	resetInstance	||	\date	2020/12/30 22:19
	//	\brief	you must call this manually prior to program exit to avoid memory leaks
	static void resetInstance();
public:
	~KeyConsole();

	//	\function	print	||	\date	2020/12/30 22:19
	//	\brief	print to stdout
	DWORD print( const std::string &msg );
	DWORD log( const std::string &msg, LogCategory cat = None );
	//	\function	error	||	\date	2020/12/30 22:19
	//	\brief	print to stderr
	DWORD error( const std::string &msg );
	//	\function	read	||	\date	2020/12/30 22:19
	//	\brief	read from stdin, returns the string
	std::string read( const uint32_t maxChars = 1024u );

	void setTitle( const std::string& title ) const;
	bool closeConsole();
	const HWND getWindowHandle() const noexcept;
	int getConsoleMode() const noexcept;
	std::string getConsoleModeStr() const noexcept;
	// get current console's Code Page. for a list of code pages check link:
	// https://docs.microsoft.com/el-gr/windows/desktop/Intl/code-page-identifiers
	const HANDLE getHandle() const noexcept;
	std::string getTitle() const noexcept;
	uint32_t getConsoleCodePage() const noexcept;
	int32_t setConsoleCodePage( const uint32_t cp );
	void setFont( const std::string &fontName );
	int32_t setCurcorPos( const _COORD xy = {0,0} );
	bool setColor( const WORD attributes = FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_GREEN );
	WORD getConsoleTextAttributes() const noexcept;
	WORD getConsoleDefaultTextAttributes() const noexcept;
	void show() const;
	void hide() const;

	inline bool operator==( const KeyConsole &rhs ) const noexcept
	{
		return this->m_fp == rhs.m_fp;
	}
	inline bool operator!=( const KeyConsole &rhs ) const noexcept
	{
		return this->m_fp != rhs.m_fp;
	}
	inline bool operator==( const KeyConsole *rhs ) const noexcept
	{
		return this->m_fp == rhs->m_fp;
	}
	inline bool operator!=( const KeyConsole *rhs ) const noexcept
	{
		return this->m_fp != rhs->m_fp;
	}
private:
	DWORD getFontFamily( const HANDLE h );
	void getConsoleInfo( const HANDLE h );
	bool setDefaultColor();
};