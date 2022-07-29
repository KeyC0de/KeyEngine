#include "gameplay_exception.h"


GameplayException::GameplayException( int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

const std::string GameplayException::getType() const noexcept
{
	return typeid( this ).name();
}

const char* GameplayException::what() const noexcept
{
	return KeyException::what();
}