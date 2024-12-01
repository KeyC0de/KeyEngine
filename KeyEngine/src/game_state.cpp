#include "game_state.h"
#include "assertions_console.h"


IState::IState( const int stateId )
	:
	m_stateId(stateId)
{

}

int IState::getStateId() const noexcept
{
	ASSERT( m_stateId != -1, "Invalid State! Must subclass `State`." );
	return m_stateId;
}

//////////////////////////////////////////////////////////////////////
MenuState::MenuState(const int stateId)
	:
	IState(stateId)
{

}

void MenuState::update()
{
	pass_;
}

//////////////////////////////////////////////////////////////////////
GameState::GameState( const int stateId )
	:
	IState(stateId)
{

}

void GameState::update()
{
	pass_;
}
