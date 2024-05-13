#pragma once

#include <string>


namespace ui
{

class Component
{
	int m_id;
	std::string m_name;
public:
	Component( const std::string &name );
	virtual ~Component() noexcept;

	int getId() const noexcept;
	std::string getName() const noexcept;
private:
	int getNextId() const noexcept;
};



}// namespace ui