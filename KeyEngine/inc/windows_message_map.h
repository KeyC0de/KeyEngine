#pragma once

#include <unordered_map>
#include "winner.h"


///=============================================================
/// \class	WindowsMessageMap
/// \author	KeyC0de
/// \date	2020/09/11 16:52
/// \brief	maps windows message ids to string ids
/// \brief	list of all known windows messages, courtesy of: https://wiki.winehq.org/List_Of_Windows_Messages
///=============================================================
class WindowsMessageMap final
{
	std::unordered_map<DWORD, std::string> m_map;

	WindowsMessageMap() noexcept;
public:
	WindowsMessageMap( const WindowsMessageMap &rhs ) = delete;
	WindowsMessageMap& operator=( const WindowsMessageMap &rhs ) = delete;

	static WindowsMessageMap& getInstance()
	{
		static WindowsMessageMap inst{};
		return inst;
	}

	std::string toString( const DWORD uMsg ) const noexcept;
};