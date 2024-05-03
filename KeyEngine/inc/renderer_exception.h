#pragma once

#include "key_exception.h"


namespace ren
{

class RendererException final
	: public KeyException
{
public:
	RendererException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

	std::string getType() const noexcept override final;
	virtual const char* what() const noexcept override final;
};


}


#define THROW_RENDERER_EXCEPTION( msg ) __debugbreak();\
	throw ren::RendererException( __LINE__, __FILE__, __FUNCTION__, msg );