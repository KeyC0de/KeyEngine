#include "gameplay_exception.h"
#include <typeinfo>


GameplayException::GameplayException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

std::string GameplayException::getType() const noexcept
{
	return typeid( this ).name();
}

const char* GameplayException::what() const noexcept
{
	return KeyException::what();
}