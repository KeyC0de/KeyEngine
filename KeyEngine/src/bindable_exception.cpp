#include "bindable_exception.h"


BindableException::BindableException( int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{

}

const std::string BindableException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* BindableException::what() const noexcept
{
	return KeyException::what();
}