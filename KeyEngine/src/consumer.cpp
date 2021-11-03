#include "consumer.h"
#include <cctype>
#include "renderer_exception.h"
#include "utils.h"
#include "assertions_console.h"


namespace ren
{

IConsumer::IConsumer( const std::string& name )
	:
	m_name{name}
{
	ASSERT( !name.empty(), "Input name is empty!" );

	const bool bValidName = std::all_of( name.begin(),
		name.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	ASSERT( bValidName && !std::isdigit( name.front() ), "Invalid Input name!" );
}

void IConsumer::setPassAndProducerNames( const std::string& passName,
	const std::string& producerName )
{
	ASSERT( !passName.empty(), "Pass name is empty!" );
	ASSERT( !producerName.empty(), "Output name is empty!" );

	const bool bPassNameValid = std::all_of( passName.begin(),
		passName.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	if ( passName != "$" && ( !bPassNameValid || std::isdigit( passName.front() ) ) )
	{
		throwRendererException( "IConsumer - " + m_name + " : Invalid pass name: " + passName );
	}
	m_passName = passName;

	const bool checkproducerName = std::all_of( producerName.begin(),
		producerName.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	if ( !checkproducerName || std::isdigit( producerName.front() ) )
	{
		throwRendererException( "IConsumer - " + m_name + " : Invalid output name: " + producerName );
	}
	m_producerName = producerName;
}

const std::string& IConsumer::getName() const noexcept
{
	return m_name;
}

const std::string& IConsumer::getPassName() const noexcept
{
	return m_passName;
}

const std::string& IConsumer::getProducerName() const noexcept
{
	return m_producerName;
}


}//ren