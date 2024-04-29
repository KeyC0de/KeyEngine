#pragma once

#include <memory>
#include <vector>
#include "non_copyable.h"


class Entity;
class Operation;

//============================================================
//	\class	Message
//	\author	KeyC0de
//	\date	2019/12/09 13:23
//	\brief	The Message - Enveloppe, move-only type
//=============================================================
class Message
	: public NonCopyable
{
public:
	enum Type
	{
		Idle = 0,
		Damage,
		Heal,
		Greet,
		PhysicsCollision
	};
private:
	bool m_bHandled = false;
	Entity *m_pSender;
	std::vector<Entity*> m_recipients;
	Message::Type m_type;
public:
	Message( Entity *srcId, const std::vector<Entity*> &destId, const Message::Type type );
	Message( Message &&rhs ) noexcept;
	Message& operator=( Message &&rhs ) noexcept;
	virtual ~Message() noexcept;

	const Message::Type getType() const noexcept;
	const Entity* getSender() noexcept;
	const std::vector<Entity*>& getRecipients() noexcept;
	bool isHandled() const noexcept;
	void setHandled( const bool b ) noexcept;
};


//============================================================
//	\class	MessageCall
//	\author	KeyC0de
//	\date	2019/12/10 3:01
//	\brief	a Message with arbitrary callable object
//=============================================================
class MessageCall
	:
	public Message
{
	std::unique_ptr<Operation> m_pFunc;
public:
	MessageCall( Entity *psrc, const std::vector<Entity*> &pDests, Message::Type type, std::unique_ptr<Operation> df );
	MessageCall( MessageCall &&rhs ) noexcept;
	MessageCall& operator=( MessageCall &&rhs ) noexcept;
	virtual ~MessageCall() noexcept = default;

	Operation* getCallable() const noexcept;
};


//============================================================
//	\class	MessageData<T>
//	\author	KeyC0de
//	\date	2019/12/10 3:01
//	\brief	a Message with arbitrary data payload T
//=============================================================
template<class T>
class MessageData
	:
	public Message
{
	std::unique_ptr<T> m_pPayload;
public:
	template<typename ... TParams>
	MessageData( Entity *src,
		const std::vector<Entity*> &pDests,
		Message::Type type,
		TParams&&... args )
		:
		Message{src, pDests, type},
		m_pPayload{std::make_unique<T>( std::forward<TParams>( args )... )}
	{

	}

	virtual ~MessageData() noexcept = default;

	MessageData( MessageData &&rhs ) noexcept
		:
		Message{std::move( rhs )},
		m_pPayload{std::move( rhs.m_pPayload )}
	{

	}

	MessageData& operator=( MessageData &&rhs ) noexcept
	{
		Message::operator=( std::move( rhs ) );
		std::swap( m_pPayload, rhs.m_pPayload );
		return *this;
	}

	T* getPayload() const noexcept
	{
		return m_pPayload.get();
	}
};