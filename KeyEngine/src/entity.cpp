#include <iostream>
#include "entity.h"
//#include "entity_manager.h"
#include "message.h"
#include "operation.h"
#include "message_queue_bus_dispatcher.h"


Entity::Entity( const EntityIndex version,
	const EntityIndex index,
	const std::string &name,
	const Category categoryId,
	Entity *pParent )
	:
	m_version(version),
	m_index(index),
	m_name(name),
	m_category(categoryId),
	m_pParent(pParent)
{

}

Entity::~Entity() noexcept
{
	//EntityManager::getInstance().recycleEntityId( getId() );
}

Entity::Entity( Entity &&rhs ) noexcept
	:
	m_version{rhs.m_version},
	m_index{rhs.m_index},
	m_name{rhs.m_name},
	m_category{rhs.m_category},
	m_pParent{rhs.m_pParent}
{
	if ( rhs.hasChildren() )
	{
		int nChildren = rhs.getChildrenCount();
		m_children.reserve( nChildren );
		for ( int i = 0; i < nChildren; ++i )
		{
			m_children[i] = rhs.m_children[i];
		}
	}
}

Entity& Entity::operator=( Entity &&rhs ) noexcept
{
	Entity tmp{std::move( rhs )};
	std::swap( *this, tmp );
	return *this;
}

EntityIndex Entity::getVersion() const noexcept
{
	return m_version;
}

EntityIndex Entity::getIndex() const noexcept
{
	return m_index;
}

void Entity::printInfo() const noexcept
{
	std::cout << "entity[index="
		<< m_index
		<< "] stored @"
		<< this
		<< ", version="
		<< m_version
		<< ", id="
		<< getId()
		<< ", category="
		<< m_category
		<< '\n';
}

EntityId Entity::getId() const noexcept
{
#ifdef _32_BIT_ENTITY
	return ( m_version << 16 ) | m_index;
#elif _64_BIT_ENTITY
	return ( (EntityId) m_version << 32 ) | m_index;
#endif
}

const std::string& Entity::getName() const noexcept
{
	return m_name;
}

Entity::Category Entity::getCategory() const noexcept
{
	return m_category;
}

Entity* Entity::getParent()
{
	return m_pParent;
}

Entity* Entity::getParent() const noexcept
{
	return m_pParent;
}

void Entity::addChild( Entity *child ) noexcept
{
	m_children.push_back( child );
}

std::vector<Entity*>& Entity::getChildren()
{
	return m_children;
}

const std::vector<Entity*>& Entity::getChildren() const noexcept
{
	return m_children;
}

bool Entity::hasChildren() const noexcept
{
	return !m_children.empty();
}

int Entity::getChildrenCount() const noexcept
{
	return (int) m_children.size();
}

void Entity::onMessageReceived( std::unique_ptr<Message> msg )
{
	auto pDataMsg = dynamic_cast<MessageCall*>( msg.get() );

	auto callback = pDataMsg->getCallable();
	(*callback)();

	switch ( pDataMsg->getType() )
	{
	case Message::Type::Damage:
	{
		//m_hp -= payload->m_damage;
		std::cout << "Damage message received" << '\n';
		break;
	}
	case Message::Type::Heal:
	{
		//m_hp += payload->m_heal;
		std::cout << "Heal message received" << '\n';
		break;
	}
	case Message::Type::Greet:
	{
		//greet( payload->m_greeting );
		std::cout << "Greet message received" << '\n';
		break;
	}
	case Message::Type::PhysicsCollision:
	{
		std::cout << "Physics collision received" << '\n';
		break;
	}
	default:
	{
		break;
	}
	}
	pDataMsg->setHandled( true );
}

void Entity::sendMessage( Message *msg ) const noexcept
{
	auto &md = MessageDispatcher::getInstance();
	md.addMessage( msg );
}

inline bool Entity::operator==( const Entity *rhs ) const noexcept
{
	return this->getId() == rhs->getId();
}

inline bool Entity::operator!=( const Entity *rhs ) const noexcept
{
	return this->getId() != rhs->getId();
}

inline bool Entity::operator==( const Entity &rhs ) const noexcept
{
	return this->getId() == rhs.getId();
}

inline bool Entity::operator!=( const Entity &rhs ) const noexcept
{
	return this->getId() != rhs.getId();
}