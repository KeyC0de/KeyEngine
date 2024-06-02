#pragma once

#include <vector>
#include <string>
#include "entity_defines.h"


class EntityManager;

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
	Entity( const EntityIndex version, const EntityIndex index, const std::string &name, const Category categoryId = UNCATEGORIZED, Entity *pParent = nullptr );

	EntityIndex getVersion() const noexcept;
	EntityIndex getIndex() const noexcept;
public:
	virtual ~Entity() noexcept;
	Entity( Entity &&rhs ) noexcept;
	Entity& operator=( Entity &&rhs ) noexcept;

	void printInfo() const noexcept;
	//	\function	id	||	\date	2019/12/09 14:05
	//	\brief	get entity id
	EntityId getId() const noexcept;
	const std::string& getName() const noexcept;
	Category getCategory() const noexcept;
	Entity* getParent();
	Entity* getParent() const noexcept;
	void addChild( Entity *child ) noexcept;
	std::vector<Entity*>& getChildren();
	const std::vector<Entity*>& getChildren() const noexcept;
	bool hasChildren() const noexcept;
	int getChildrenCount() const noexcept;
	void onMessageReceived( std::unique_ptr<class Message> msg );
	void sendMessage( class Message *msg ) const noexcept;
	inline bool operator==( const Entity *rhs ) const noexcept;
	inline bool operator!=( const Entity *rhs ) const noexcept;
	inline bool operator==( const Entity &rhs ) const noexcept;
	inline bool operator!=( const Entity &rhs ) const noexcept;
};