#include "entity_manager.h"
#include "entity.h"
#include "gameplay_exception.h"


EntityManager::Bucket::Bucket( int categoryId )
	:
	m_categoryId(categoryId)
{
	// assert categoryId is non 0
}

int EntityManager::Bucket::getCategoryId() const noexcept
{
	return m_categoryId;
}

void EntityManager::Bucket::appendEntity( Entity *pEnt )
{
	m_pEntities.emplace_back( pEnt );
}

void EntityManager::Bucket::removeEntity( Entity *pEnt ) cond_noex
{
	auto it = std::find( m_pEntities.begin(), m_pEntities.end(), pEnt );
	m_pEntities.erase( it );
}

EntityManager::EntityManager()
{
	m_entities.reserve( 1000 );
}

EntityManager& EntityManager::getInstance()
{
	if ( s_pInstance == nullptr )
	{
		s_pInstance = new EntityManager{};
	}
	return *s_pInstance;
}

void EntityManager::resetInstance()
{
	if ( s_pInstance != nullptr )
	{
		delete s_pInstance;
	}
}

EntityId EntityManager::spawnEntity( const std::string &name,
	int categoryId,
	Entity *pParent )
{
	EntityIndex index;
	if ( m_freelist.empty() )
	{
		index = static_cast<EntityIndex>( m_entities.size() );
		auto p = std::make_unique<Entity>( Entity{1, index, name, static_cast<Entity::Category>( categoryId ), pParent} );
		m_entities.emplace_back( std::move( p ) );

		// If the entity is of a specific category add it to the appropriate Bucket
		switch( categoryId )
		{
		case 1:
		{
			m_statics.appendEntity( p.get() );
			break;
		}
		case 2:
		{
			m_bullets.appendEntity( p.get() );
			break;
		}
		case 3:
		{
			m_ragDolls.appendEntity( p.get() );
			break;
		}
		case 4:
		{
			m_npcs.appendEntity( p.get() );
			break;
		}
		case 5:
		{
			m_mounts.appendEntity( p.get() );
			break;
		}
		default :	// UNCATEGORIZED - 0
		{
			break;
		}
		}//switch
	}
	else	// if there are dead entities
	{
		index = m_freelist.back();
		//m_entities[index].push_back( Entity() );
		//m_freelist.push_back( m_entities.size() - 1 );
		m_freelist.pop_back();
	}
	return ( m_entities[index]->m_version << 16 ) | index;
}

EntityIndex EntityManager::getAliveEntityCount()
{
	return static_cast<EntityIndex>( m_entities.size() - m_freelist.size() );
}

Entity *EntityManager::getEntityById( EntityId entId )
{
	EntityIndex index = entId & 0xFFFF;
	EntityIndex version = entId >> 16;
	if ( index < m_entities.size()
		&& m_entities[index]->m_version == version )
	{
		return m_entities[index].get();
	}
	return nullptr;
}

void EntityManager::recycleEntityId( EntityId entId )
{
	EntityIndex index = entId & 0xFFFF;
	++m_entities[index]->m_version;
	m_freelist.emplace_back( index );
}

EntityManager::Bucket& EntityManager::getBucket( int categoryId )
{
	switch( categoryId )
	{
	case 1:
	{
		return m_statics;
		break;
	}
	case 2:
	{
		return m_bullets;
		break;
	}
	case 3:
	{
		return m_ragDolls;
		break;
	}
	case 4:
	{
		return m_npcs;
		break;
	}
	case 5:
	{
		return m_mounts;
		break;
	}
	default :	// UNCATEGORIZED - 0
	{
		THROW_GAMEPLAY_EXCEPTION( "Entity doesn't belong in a bucket." );
	}
	}//switch
}

Entity* EntityManager::world()
{
	// current world Entity index is always @ index 0 of m_worldEntitiesIndices
	return m_entities[m_worldEntitiesIndices[0]].get();
}