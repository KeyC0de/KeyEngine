#pragma once

#include "key_exception.h"


class BindableException final
	: public KeyException
{
public:
	BindableException( int line, const char* file, const char* function,
		const std::string& msg ) noexcept;

	const std::string getType() const noexcept override;
	virtual const char* what() const noexcept override;
};


#define throwBindableException( msg ) throw BindableException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );