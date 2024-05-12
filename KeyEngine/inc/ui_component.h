#pragma once

#include <string>


namespace ui
{

class Component
{
	int m_hierarchyId;
	std::string m_id;
public:
	Component( const std::string &name );
	virtual ~Component() noexcept;

	int getHierarchyId() const noexcept;
	std::string getName() const noexcept;
	std::string getId() const noexcept;
private:
	int getNextId() const noexcept;
};



}// namespace ui