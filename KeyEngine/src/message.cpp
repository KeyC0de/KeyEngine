#include "message.h"
#include "operation.h"
#include "entity.h"


Message::Message( Entity *pSrc,
	const std::vector<Entity*> &pDests,
	const Message::Type type )
	:
	m_pSender(pSrc),
	m_type(type)
{
	for( Entity *dest : pDests )
	{
		m_recipients.push_back( dest );
	}
}

Message::~Message() noexcept
{
	//m_recipients.clear();
}

Message::Message( Message &&rhs ) noexcept
	:
	m_bHandled{rhs.m_bHandled},
	m_pSender{rhs.m_pSender},
	m_type{rhs.m_type}
{
	for( Entity *dest : rhs.m_recipients )
	{
		m_recipients.push_back( dest );
	}
	rhs.m_recipients.clear();
}

Message& Message::operator=( Message &&rhs ) noexcept
{
	Message tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

const Message::Type Message::getType() const noexcept
{
	return m_type;
}

const Entity* Message::getSender() noexcept
{
	return m_pSender;
}

const std::vector<Entity*>& Message::getRecipients() noexcept
{
	return m_recipients;
}

bool Message::isHandled() const noexcept
{
	return m_bHandled;
}

void Message::setHandled( const bool b ) noexcept
{
	m_bHandled = b;
}


MessageCall::MessageCall( Entity *psrc,
	const std::vector<Entity*> &pDests,
	Message::Type type,
	std::unique_ptr<Operation> df )
	:
	Message{psrc, pDests, type},
	m_pFunc{std::move( df )}
{

}

MessageCall::MessageCall( MessageCall &&rhs ) noexcept
	:
	Message{std::move( rhs )},
	m_pFunc{std::move( rhs.m_pFunc )}
{

}

MessageCall& MessageCall::operator=( MessageCall &&rhs ) noexcept
{
	Message::operator=( std::move( rhs ) );
	std::swap( m_pFunc, rhs.m_pFunc );
	return *this;
}

Operation* MessageCall::getCallable() const noexcept
{
	return m_pFunc.get();
}