#include "binder.h"
#include <cctype>
#include "renderer_exception.h"
#include "utils.h"
#include "assertions_console.h"


namespace ren
{

IBinder::IBinder( const std::string &name )
	:
	m_name{name}
{
	ASSERT( !name.empty(), "Input name is empty!" );

	const bool bValidName = std::all_of( name.begin(), name.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	ASSERT( bValidName && !std::isdigit( name.front() ), "Invalid Input name!" );
}

void IBinder::setPassAndLinkerNames( const std::string &passName,
	const std::string &linkerName )
{
	ASSERT( !passName.empty(), "Pass name is empty!" );
	ASSERT( !linkerName.empty(), "Output name is empty!" );

	const bool bPassNameValid = std::all_of( passName.begin(), passName.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	if ( passName != "$" && ( !bPassNameValid || std::isdigit( passName.front() ) ) )
	{
		THROW_RENDERER_EXCEPTION( "IBinder - " + m_name + " : Invalid pass name: " + passName );
	}
	m_passName = passName;

	const bool checkLinkerName = std::all_of( linkerName.begin(), linkerName.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	if ( !checkLinkerName || std::isdigit( linkerName.front() ) )
	{
		THROW_RENDERER_EXCEPTION( "IBinder - " + m_name + " : Invalid output name: " + linkerName );
	}
	m_linkerName = linkerName;
}

const std::string& IBinder::getName() const noexcept
{
	return m_name;
}

const std::string& IBinder::getPassName() const noexcept
{
	return m_passName;
}

const std::string& IBinder::getLinkerName() const noexcept
{
	return m_linkerName;
}


}//namespace ren