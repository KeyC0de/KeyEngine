#include "renderer_exception.h"
#include "winner.h"
#include <typeinfo>


namespace ren
{

RendererException::RendererException( int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException(line, file, function, msg)
{
	OutputDebugStringA( what() );
}

const char* RendererException::what() const noexcept
{
	return KeyException::what();
}

const std::string RendererException::getType() const noexcept
{
	return typeid( *this ).name();
}


}