#include "game_state.h"
#include "assertions_console.h"


State::State( const int stateId )
	:
	m_stateId(stateId)
{

}

int State::getStateId() const noexcept
{
	ASSERT( m_stateId != -1, "Invalid State. Must subclass 'State'" );
	return m_stateId;
}


MenuState::MenuState( const int stateId )
	:
	State(stateId)
{

}

GameState::GameState( const int stateId )
	:
	State(stateId)
{

}