#pragma once

#include "key_exception.h"


class UtilException final
	: public KeyException
{
public:
	UtilException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

	std::string getType() const noexcept override final;
	virtual const char* what() const noexcept override final;
};


#define THROW_UTIL_EXCEPTION( msg ) throw UtilException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );\
	__debugbreak();