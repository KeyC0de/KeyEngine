#pragma once

#include "key_exception.h"


class BindableException final
	: public KeyException
{
public:
	BindableException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

	std::string getType() const noexcept override final;
	virtual const char* what() const noexcept override final;
};


#define THROW_BINDABLE_EXCEPTION( msg ) __debugbreak();\
	throw BindableException( __LINE__, __FILE__, __FUNCTION__, msg );