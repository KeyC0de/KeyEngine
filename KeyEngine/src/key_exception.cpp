#include <sstream>
#include "key_exception.h"


KeyException::KeyException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	m_line(line),
	m_function( function ),
	m_file(file),
	m_description(msg)
{

}

#pragma warning( disable : 4129 )
const char* KeyException::what() const noexcept
{
	using namespace std::string_literals;
	std::ostringstream oss;
	oss << "\n\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ \nException "s
		<< getType()
		<< "\" occurred in '"s
		<< m_file
		<< "' - "s
		<< m_function
		<< "() @ line:"s
		<< std::to_string( m_line )
		<< "\nDescription: "s
		<< m_description
		<< "\n\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ \n"s
		<< std::endl;
	m_description.clear();
	m_description.assign( oss.str() );
	return m_description.data();
}
#pragma warning( default : 4129 )

std::string KeyException::getType() const noexcept
{
	return typeid( this ).name();
}

unsigned KeyException::getLine() const noexcept
{
	return m_line;
}

const std::string& KeyException::getFile() const noexcept
{
	return m_file;
}

const std::string& KeyException::getFunction() const noexcept
{
	return m_function;
}