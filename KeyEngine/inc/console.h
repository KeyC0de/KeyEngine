#pragma once

#include <string>
#include "winner.h"
#include "non_copyable.h"


//============================================================
//	\class	KeyConsole
//
//	\author	KeyC0de
//	\date	2020/07/10 15:10
//
//	\brief	interface to the process's console
//			Singleton class
//			by default set to output mode
//			the mode is changed dynamically depending on print/log/read operation request
//=============================================================
class KeyConsole final
	: public NonCopyable
{
	static inline constexpr const char *currentVersion = "v0.4";
	static inline constexpr const char *defaultConsoleTitle = "Debug Console - ";
	static inline KeyConsole *m_pInstance;
	FILE *m_fp;
	std::string m_title;
	DWORD m_stdDevice;
	FILE *m_hMode;			// set this when you print/log/read to stdout/stderr/stdin
	HANDLE m_hConsole;
	WORD m_consoleAttributesDefault;
	WORD m_consoleAttributes;

	KeyConsole( const std::string &fontName = "Lucida Console" );
	bool setDefaultColor();
public:
	~KeyConsole();

	static const DWORD getFontFamily( const HANDLE h );
	static void getConsoleInfo( const HANDLE h );
public:
	bool closeConsole();
	static KeyConsole& getInstance() noexcept;
	//===================================================
	//	\function	resetInstance
	//	\brief  you must call this manually prior to program exit to avoid memory leaks
	//	\date	2020/12/30 22:19
	static void resetInstance();
	const int getConsoleMode() const noexcept;
	const std::string getConsoleModeStr() const noexcept;
	// get current console's Code Page. for a list of code pages check link:
	// https://docs.microsoft.com/el-gr/windows/desktop/Intl/code-page-identifiers
	const HANDLE getHandle() const noexcept;
	const uint32_t getConsoleCodePage() const noexcept;
	const int32_t setConsoleCodePage( const uint32_t cp );
	void setFont( const std::string &fontName );
	const int32_t setCurcorPos( const _COORD xy = {0,0} );
	bool setColor( const WORD attributes = FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_GREEN );
	const WORD getConsoleTextAttributes() const noexcept;
	const WORD getConsoleDefaultTextAttributes() const noexcept;
	void show() const;
	void hide() const;
	//===================================================
	//	\function	print
	//	\brief  print to stdout
	//	\date	2020/12/01 21:36
	DWORD print( const std::string &msg );
	//===================================================
	//	\function	log
	//	\brief  print to stderr
	//	\date	2020/12/01 21:36
	DWORD log( const std::string &msg );
	//===================================================
	//	\function	read
	//	\brief  read from stdin, returns the string
	//	\date	2020/12/01 21:36
	std::string read( const uint32_t maxChars = 1024u );

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
};