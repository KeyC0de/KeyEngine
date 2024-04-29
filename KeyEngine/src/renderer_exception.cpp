#include "renderer_exception.h"
#include "winner.h"
#include <typeinfo>


namespace ren
{

RendererException::RendererException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{

}

const char* RendererException::what() const noexcept
{
	return KeyException::what();
}

std::string RendererException::getType() const noexcept
{
	return typeid( *this ).name();
}


}