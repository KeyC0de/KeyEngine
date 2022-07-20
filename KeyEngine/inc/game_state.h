#pragma once


class Graphics;

class State
{
protected:
	int m_stateId = -1;

	State( int stateId = -1 );
public:
	virtual ~State() noexcept;

	virtual void update() = 0;
	int getStateId() const noexcept;
};

class MenuState final
	: public State
{
public:
	explicit MenuState( int stateId = 0 );
	virtual ~MenuState() noexcept override;

	void update() override;
};

class GameState final
	: public State
{
public:
	explicit GameState( int stateId = 1 );
	virtual ~GameState() noexcept override;

	void update() override;
};