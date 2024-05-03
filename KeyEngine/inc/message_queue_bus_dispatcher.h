#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "non_copyable.h"


class Message;

//============================================================
//	\class	MessageBus
//	\author	KeyC0de
//	\date	2019/12/09 4:51
//	\brief	thread safe event queue built from a Vector
//			enqueue or dequeue whenever, wherever, we'll be together
//			it pa-owns its contents (Messages)
//			move only
//			accessible only from the MessageDispatcher friend class
//=============================================================
class MessageBus final
	: public NonCopyable
{
	friend class MessageDispatcher;

	// #TODO: Change vector of unique_ptr to Message to vector of objects of type Message
	std::vector<std::unique_ptr<Message>> m_vec;
	std::size_t m_size = 0;
	mutable std::mutex m_mu;
	std::condition_variable m_cond;
private:
	MessageBus() = default;
	MessageBus( const int initialCapacity );
private:
	void removeFrontByBackSwap();
public:
	MessageBus( MessageBus &&rhs ) noexcept;
	MessageBus& operator=( MessageBus &&rhs ) noexcept;

	//	\function	enqueue	||	\date	2019/12/09 4:50
	//	\brief	push_back() : enqueue at the back
	void enqueue( Message *msg );
	//	\function	dequeue	||	\date	2019/12/09 4:51
	//	\brief	pop_front() : dequeue messages from the front
	std::unique_ptr<Message> dequeue();
	const Message* peekFront() const noexcept;
	const Message* peekBack() const noexcept;
	explicit operator bool();
	Message* operator[]( const std::size_t index );
	const Message* operator[]( const std::size_t index ) const;
	std::size_t getSize() const noexcept;
	std::size_t getCapacity() const noexcept;
	bool isEmpty() const noexcept;
	void clear();
};


//============================================================
//	\class	MessageDispatcher
//	\author	KeyC0de
//	\date	2019/12/09 17:15
//	\brief	Meyer's singleton class
//			owns & manages the MessageBus/Queue
//=============================================================
class MessageDispatcher final
	: public NonCopyable
{
	MessageBus m_mb;
private:
	MessageDispatcher( const int initialCapacity );
public:
	~MessageDispatcher() noexcept = default;
	MessageDispatcher( MessageDispatcher &&rhs ) noexcept;
	MessageDispatcher& operator=( MessageDispatcher &&rhs ) noexcept;

	static MessageDispatcher& getInstance( const int initialCapacity = 100 );
	//	\function	addMessage	||	\date	2020/12/10 4:47
	//	\brief	add new message to the MessageBus
	void addMessage( Message *msg );
	//	\function	dispatchAll	||	\date	2019/12/10 4:47
	//	\brief	dispatch all pending messages
	void dispatchAll();
	// #TODO: dispatchByEventType
	// #TODO: dispatchEventsTargetedTo( specific actor )
	void clear();

	std::size_t getSize() const noexcept;
	std::size_t getCapacity() const noexcept;
};