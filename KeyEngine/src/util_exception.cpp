#include "util_exception.h"
#include <typeinfo>


UtilException::UtilException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{

}

std::string UtilException::getType() const noexcept
{
	return typeid( *this ).name();
}

const char* UtilException::what() const noexcept
{
	return KeyException::what();
}