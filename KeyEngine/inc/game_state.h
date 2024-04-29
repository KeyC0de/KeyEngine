#pragma once


class State
{
protected:
	int m_stateId = -1;

	State( const int stateId = -1 );
public:
	virtual ~State() noexcept = default;

	virtual void update() = 0;
	int getStateId() const noexcept;
};

class MenuState final
	: public State
{
public:
	explicit MenuState( const int stateId = 0 );

	void update() override
	{
		pass_;
	}
};

class GameState final
	: public State
{
public:
	explicit GameState( const int stateId = 1 );

	void update() override
	{
		pass_;
	}
};