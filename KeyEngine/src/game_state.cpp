#include "game_state.h"
#include "graphics.h"
#include "assertions_console.h"


State::State( int stateId )
	:
	m_stateId( stateId )
{

}

State::~State() noexcept
{

}

int State::getStateId() const noexcept
{
	ASSERT( m_stateId != -1,
		"Invalid State. Must subclass 'State'" );
	return m_stateId;
}


MenuState::MenuState( int stateId )
	:
	State(stateId)
{

}

MenuState::~MenuState() noexcept
{

}

void MenuState::update()
{

}

void MenuState::render( Graphics* gph ) const
{

}


GameState::GameState( int stateId )
	:
	State(stateId)
{

}

GameState::~GameState() noexcept
{

}

void GameState::update()
{

}

void GameState::render( Graphics* gph ) const
{

}