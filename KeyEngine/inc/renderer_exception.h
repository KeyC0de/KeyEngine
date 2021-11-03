#pragma once

#include "key_exception.h"


namespace ren
{

class RendererException final
	: public KeyException
{
public:
	RendererException( int line, const char* file, const char* function,
		const std::string& msg ) noexcept;

	const std::string getType() const noexcept override;
	virtual const char* what() const noexcept override;
};


}


#define throwRendererException( msg ) throw ren::RendererException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );