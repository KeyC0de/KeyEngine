#include "message_queue_bus_dispatcher.h"
#include "message.h"
#include "entity.h"
#include "assertions_console.h"


void MessageBus::removeFrontByBackSwap()
{
	std::swap( m_vec.front(), m_vec.back() );
	m_vec.pop_back();
}

MessageBus::MessageBus( const int initialCapacity )
	:
	m_vec(initialCapacity)
{

}

MessageBus::MessageBus( MessageBus &&rhs ) noexcept
	:
	m_vec{std::move( rhs.m_vec )},
	m_size(rhs.m_size)
{

}

MessageBus& MessageBus::operator=( MessageBus &&rhs ) noexcept
{
	MessageBus tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

void MessageBus::enqueue( Message *msg )
{
	std::unique_lock<std::mutex> lg{m_mu};
	m_vec.emplace_back( msg );
	++m_size;
	m_cond.notify_one();
}

std::unique_ptr<Message> MessageBus::dequeue()
{
	std::unique_lock<std::mutex> ul{m_mu};

	if ( m_vec.empty() )
	{
		throw std::runtime_error{"Attempted to pop from empty Queue."};
	}
	while ( m_vec.empty() )
	{
		m_cond.wait( ul );
	}
	std::unique_ptr<Message> pOut = std::move( m_vec.back() );
	m_vec.pop_back();
	--m_size;
	return pOut;
}

const Message* MessageBus::peekFront() const noexcept
{
	std::lock_guard<std::mutex> lg{m_mu};
	return m_vec.front().get();
}

const Message* MessageBus::peekBack() const noexcept
{
	std::lock_guard<std::mutex> lg{m_mu};
	return m_vec.back().get();
}

MessageBus::operator bool()
{
	std::lock_guard<std::mutex> lg{m_mu};
	return !m_vec.empty();
}

Message* MessageBus::operator[]( const std::size_t index )
{
	std::lock_guard<std::mutex> lg{m_mu};
	if ( m_vec.empty() || index < 0 || index >= m_vec.size() )
	{
		throw std::out_of_range( "Invalid index." );
	}
	return m_vec[index].get();
}

const Message* MessageBus::operator[]( const std::size_t index ) const
{
	std::lock_guard<std::mutex> lg{m_mu};
	if ( m_vec.empty() || index < 0 || index >= m_vec.size() )
	{
		throw std::out_of_range( "Invalid index." );
	}
	return m_vec[index].get();
}

bool MessageBus::isEmpty() const noexcept
{
	return m_vec.empty();
}

void MessageBus::clear()
{
	m_vec.clear();
}

std::size_t MessageBus::getSize() const noexcept
{
	return m_size;
}

std::size_t MessageBus::getCapacity() const noexcept
{
	return m_vec.capacity();
}


MessageDispatcher::MessageDispatcher( const int initialCapacity )
	:
	m_mb(initialCapacity)
{

}

MessageDispatcher::MessageDispatcher( MessageDispatcher &&rhs ) noexcept
	:
	m_mb{std::move( rhs.m_mb )}
{

}

MessageDispatcher& MessageDispatcher::operator=( MessageDispatcher &&rhs ) noexcept
{
	MessageDispatcher tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

MessageDispatcher& MessageDispatcher::getInstance( const int initialCapacity )
{
	static MessageDispatcher instance{initialCapacity};
	return instance;
}

void MessageDispatcher::addMessage( Message *msg )
{
	m_mb.enqueue( msg );
}

void MessageDispatcher::dispatchAll()
{
	while ( m_mb.getSize() > 0 )
	{
		auto msg = m_mb.dequeue();
		auto& recipients = msg->getRecipients();
		ASSERT( recipients.size() != 0, "No recipients!" );
		for ( Entity *entity : recipients )
		{
			entity->onMessageReceived( std::move( msg ) );
		}
	}
}

std::size_t MessageDispatcher::getSize() const noexcept
{
	return m_mb.getSize();
}

std::size_t MessageDispatcher::getCapacity() const noexcept
{
	return m_mb.getCapacity();
}

void MessageDispatcher::clear()
{
	m_mb.clear();
}