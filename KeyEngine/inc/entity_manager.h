#pragma once

#include <vector>
#include <memory>
#include "entity_defines.h"


class Entity;

//============================================================
//	\class	Entity
//
//	\author	KeyC0de
//	\date	2019/12/09 16:06
//
//	\brief	EntityManager owns all the Entities
//=============================================================
class EntityManager final
{
	static inline EntityManager* m_pInstance;
private:
	std::vector<std::unique_ptr<Entity>> m_entities;
	std::vector<EntityIndex> m_freelist;
	std::vector<size_t> m_worldEntitiesIndices;

	// Organize (Categorized) Entities of the same type into `Bucket`s
	// useful for various gameplay operations
	class Bucket final
	{
		int m_categoryId;
		// potentially other details
		std::vector<Entity*> m_pEntities;
	public:
		Bucket( int categoryId = 0 );

		const int getCategoryId() const noexcept;
		void appendEntity( Entity* pEnt );
		void removeEntity( Entity* pEnt ) cond_noex;
		//void sort();
	};
	Bucket m_statics{1};
	Bucket m_bullets{2};
	Bucket m_ragDolls{3};
	Bucket m_npcs{4};
	Bucket m_mounts{5};
	Bucket m_cleanup{6};		// entities for clean up..
private:
	EntityManager();
public:
	EntityManager( const EntityManager& rhs ) = delete;
	EntityManager& operator=( const EntityManager& rhs ) = delete;

	static EntityManager& getInstance();
	static void resetInstance();

	//===================================================
	//	\function	spawnEntity
	//	\brief  factory function for entities
	//	\date	2020/12/09 14:14
	EntityId spawnEntity( const std::string& name, int categoryId = 0,
		Entity* pParent = nullptr );
	EntityIndex getAliveEntities();

	//===================================================
	//	\function	getEntityById
	//	\brief  also checks if the entity is valid if its not valid (has died) returns nullptr
	//	\date	2019/12/09 14:04
	Entity* getEntityById( EntityId entId );

	// TODO: Add relinquishEntity( EntityId entId );

	//===================================================
	//	\function	recycleEntityId
	//	\brief  recycle the entity's index/id st the slot can be used again
	//			NOT WORKING ATM
	//	\date	2019/12/09 13:43
	void recycleEntityId( EntityId entId );

	Bucket& getBucket( int categoryId );

	Entity* getCurrentWorld();
};