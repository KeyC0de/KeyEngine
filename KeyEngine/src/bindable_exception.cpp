#include "bindable_exception.h"
#include <typeinfo>


BindableException::BindableException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{

}

std::string BindableException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* BindableException::what() const noexcept
{
	return KeyException::what();
}