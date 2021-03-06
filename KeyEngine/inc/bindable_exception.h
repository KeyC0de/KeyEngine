#pragma once

#include "key_exception.h"


class BindableException final
	: public KeyException
{
public:
	BindableException( int line, const char *file, const char *function,
		const std::string &msg ) noexcept;

	const std::string getType() const noexcept override final;
	virtual const char* what() const noexcept override final;
};


#define THROW_BINDABLE_EXCEPTION( msg ) throw BindableException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );