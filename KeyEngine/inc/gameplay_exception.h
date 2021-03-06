#pragma once

#include "key_exception.h"


class GameplayException final
	: public KeyException
{
public:
	GameplayException( int line, const char *file, const char *function,
		const std::string &msg ) noexcept;

	const std::string getType() const noexcept override final;
	virtual const char* what() const noexcept override final;
};


#define THROW_GAMEPLAY_EXCEPTION( msg ) throw GameplayException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );