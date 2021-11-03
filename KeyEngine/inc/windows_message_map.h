#pragma once

#include <unordered_map>
#include "winner.h"


//=============================================================
// \class	WindowsMessageMap
//
// \author	KeyC0de
// \date	2020/09/11 16:52
//
// \brief	maps windows message ids to string ids
//=============================================================
class WindowsMessageMap final
{
	std::unordered_map<DWORD, std::string> m_map;
public:
	WindowsMessageMap() noexcept;
	std::string toString( DWORD msg, LPARAM lp, WPARAM wp ) const noexcept;
};