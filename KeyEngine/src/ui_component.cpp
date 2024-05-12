#include "ui_component.h"


namespace ui
{

static int g_numComponents = 0u;

Component::Component( const std::string &name )
	:
	m_hierarchyId{getNextId()},
	m_id{name}
{

}

Component::~Component() noexcept
{

}

int Component::getHierarchyId() const noexcept
{
	return m_hierarchyId;
}

std::string Component::getName() const noexcept
{
	return m_id;
}

std::string Component::getId() const noexcept
{
	return m_id;
}

int Component::getNextId() const noexcept
{
	++g_numComponents;
	return g_numComponents;
}


}// namespace ui