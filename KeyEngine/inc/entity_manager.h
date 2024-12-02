#pragma once

#include <vector>
#include <memory>
#include <string>
#include "entity_defines.h"
#include "non_copyable.h"


class Entity;

///============================================================
/// \class	EntityManager
/// \author	KeyC0de
/// \date	2019/12/09 16:06
/// \brief	EntityManager owns all the Entities
///=============================================================
class EntityManager final
	: public NonCopyable
{
	static inline EntityManager *s_pInstance;

	std::vector<std::unique_ptr<Entity>> m_entities;
	std::vector<EntityIndex> m_freelist;
	std::vector<size_t> m_worldEntitiesIndices;

	///=============================================================
	/// \class	Bucket
	/// \author	KeyC0de
	/// \date	2024/11/21 14:27
	/// \brief	Organize (Categorized) Entities of the same type into `Bucket`s
	/// \brief	useful for various gameplay operations
	///=============================================================
	class Bucket final
	{
		int m_categoryId;
		// potentially other details
		std::vector<Entity*> m_pEntities;
	public:
		Bucket( int categoryId );

		int getCategoryId() const noexcept;
		void appendEntity( Entity *pEnt );
		void removeEntity( Entity *pEnt ) cond_noex;
		//void sort();
	};
	Bucket m_statics{1};
	Bucket m_bullets{2};
	Bucket m_ragDolls{3};
	Bucket m_npcs{4};
	Bucket m_mounts{5};
public:
	static EntityManager& getInstance();
	static void resetInstance();
	/// \brief	factory function for entities
	EntityId spawnEntity( const std::string &name, int categoryId, Entity *pParent = nullptr );
	EntityIndex getAliveEntityCount();
	/// \brief	also checks if the entity is valid if its not valid (has died) returns nullptr
	Entity* getEntityById( EntityId entId );
	/// \brief	recycle the entity's index/id st the slot can be used DOain
	/// \brief	#TODO: NOT WORKING ATM
	void recycleEntityId( EntityId entId );
	Bucket& getBucket( int categoryId );
	/// \brief	gets the current world
	Entity* world();
private:
	EntityManager();
};