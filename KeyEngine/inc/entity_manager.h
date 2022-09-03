#pragma once

#include <vector>
#include <memory>
#include <string>
#include "entity_defines.h"
#include "non_copyable.h"


class Entity;

//============================================================
//	\class	EntityManager
//
//	\author	KeyC0de
//	\date	2019/12/09 16:06
//
//	\brief	EntityManager owns all the Entities
//=============================================================
class EntityManager final
	: public NonCopyable
{
	static inline EntityManager *m_pInstance;
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
		Bucket( int categoryId );

		const int getCategoryId() const noexcept;
		void appendEntity( Entity *pEnt );
		void removeEntity( Entity *pEnt ) cond_noex;
		//void sort();
	};
	Bucket m_statics{1};
	Bucket m_bullets{2};
	Bucket m_ragDolls{3};
	Bucket m_npcs{4};
	Bucket m_mounts{5};
private:
	EntityManager();
public:
	static EntityManager& instance();
	static void resetInstance();
	//===================================================
	//	\function	spawnEntity
	//	\brief  factory function for entities
	//	\date	2020/12/09 14:14
	EntityId spawnEntity( const std::string &name, int categoryId, Entity *pParent = nullptr );
	EntityIndex getAliveEntityCount();
	//===================================================
	//	\function	getEntityById
	//	\brief  also checks if the entity is valid if its not valid (has died) returns nullptr
	//	\date	2019/12/09 14:04
	Entity* getEntityById( EntityId entId );
	//===================================================
	//	\function	recycleEntityId
	//	\brief  recycle the entity's index/id st the slot can be used again
	//			#TODO: NOT WORKING ATM
	//	\date	2019/12/09 13:43
	void recycleEntityId( EntityId entId );
	Bucket& getBucket( int categoryId );
	//===================================================
	//	\function	world
	//	\brief  gets the current world
	//	\date	2022/08/28 13:06
	Entity* world();
};