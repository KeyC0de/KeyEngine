#pragma once

#include <vector>
#include <string>
#include "entity_defines.h"


//============================================================
//	\class	Entity
//	\author	KeyC0de
//	\date	2019/12/09 13:50
//	\brief	a Game Object base class
//			Transformation: ConcreteEntity <- Entity <- Model <- Node <- ConcreteMesh <- Mesh
//=============================================================
class Entity
{
	friend class EntityManager;
	// a version tag which will be unique (inside an aggregate) for all objects that ever occupied that index
	// = 1 initially (but it doesn't matter)
	EntityIndex m_version;
	// an index unique amongst all live m_entities
	EntityIndex m_index;

	enum Category
	{
		UNCATEGORIZED,
		STATIC,
		BULLET,
		RAGDOLL,	// or dead bodies
		NPC,
		MOUNT,
		CLEANUP,
	} m_category;

	Entity *m_pParent = nullptr;
	std::string m_name;
	std::vector<Entity*> m_children;
private:
	Entity( EntityIndex version, EntityIndex index, const std::string &name, Category categoryId = UNCATEGORIZED, Entity *pParent = nullptr );

	const EntityIndex getVersion() const noexcept;
	const EntityIndex getIndex() const noexcept;
public:
	virtual ~Entity() noexcept;
	Entity( Entity &&rhs ) noexcept;
	Entity& operator=( Entity &&rhs ) noexcept;

	void printInfo() const noexcept;
	//	\function	id	||	\date	2019/12/09 14:05
	//	\brief	get entity id
	EntityId getId() const noexcept;
	const std::string& getName() const noexcept;
	const Category getCategory() const noexcept;
	Entity* parent() const noexcept;
	const Entity* getParent() const noexcept;
	void addChild( Entity *child ) noexcept;
	std::vector<Entity*>& children() noexcept;
	const std::vector<Entity*>& getChildren() const noexcept;
	bool hasChildren() const noexcept;
	const int getChildrenCount() const noexcept;
	void onMessageReceived( std::unique_ptr<class Message> msg );
	void sendMessage( class Message *msg ) const noexcept;
	inline bool operator==( const Entity *rhs ) const noexcept;
	inline bool operator!=( const Entity *rhs ) const noexcept;
	inline bool operator==( const Entity &rhs ) const noexcept;
	inline bool operator!=( const Entity &rhs ) const noexcept;
};