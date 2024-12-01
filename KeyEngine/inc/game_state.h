#pragma once


///=============================================================
/// \class	IState
/// \author	KeyC0de
/// \date	2020/11/30 15:11
/// \brief	can represents one of various types of States. The Game can be at a single concrete IState subclass at a time.
///=============================================================
class IState
{
protected:
	int m_stateId = -1;

	IState( const int stateId = -1 );
public:
	virtual ~IState() noexcept = default;

	virtual void update() = 0;
	int getStateId() const noexcept;
};

class MenuState final
	: public IState
{
public:
	explicit MenuState( const int stateId = 0 );

	void update() override;
};

class GameState final
	: public IState
{
public:
	explicit GameState( const int stateId = 1 );

	void update() override;
};