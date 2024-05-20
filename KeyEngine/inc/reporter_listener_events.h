#pragma once

#include <string>


class Graphics;

namespace gui
{

class Component;

}


struct SwapChainResizedEvent
{
	Graphics &gfx;
};

struct UISoundEvent
{
	enum SoundType
	{
		Component_Hovered,
		Component_Unhovered,
	};
	SoundType m_soundType;

	static const char* getSoundPath( SoundType soundType ) noexcept
	{
		switch ( soundType )
		{
		case Component_Hovered:
			return "assets/sfx/611451__eminyildirim__ui-click-menu-hover.wav";
		case Component_Unhovered:
			return "";
		}

		return "";
	}
};

struct UserPropertyChanged
{
	gui::Component* comp;
	std::string propertyName;
};

//=============================================================
//	\class	UiMsg
//	\author	KeyC0de
//	\date	2024/05/16 19:58
//	\brief	This event is used to make the UI event-driven, rather than having to constantly update to keep information up to date. Use it in classes that want to listen for a UI update.
//			For example we might want a Component to play an animation when there's a level-up, which this allows us to do without any additional code.
//=============================================================
struct UiMsg
{
private:
	std::string m_msgId;	// id of ui message
public:
	UiMsg() = default;
 
	UiMsg( const std::string &msg_id )
		:
		m_msgId(msg_id)
	{
	}
 
	UiMsg( const char *msg_id )
		:
		m_msgId(msg_id)
	{
	}
 
	const std::string& id() const noexcept
	{
		return m_msgId;
	}
};