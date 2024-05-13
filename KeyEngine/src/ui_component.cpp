#include "ui_component.h"


namespace ui
{

static int g_numComponents = 0u;

Component::Component( const std::string &name )
	:
	m_id{getNextId()},
	m_name{name}
{

}

Component::~Component() noexcept
{

}

int Component::getId() const noexcept
{
	return m_id;
}

std::string Component::getName() const noexcept
{
	return m_name;
}

int Component::getNextId() const noexcept
{
	++g_numComponents;
	return g_numComponents;
}


}// namespace ui