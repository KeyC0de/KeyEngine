#include "producer.h"
#include <algorithm>
#include <cctype>
#include "assertions_console.h"


namespace ren
{

IProducer::IProducer( const std::string &name )
	:
	m_name{name}
{
	ASSERT( !name.empty(), "Producer name is null!" );

	const bool bNameValid = std::all_of( name.begin(),
		name.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	ASSERT( bNameValid && !std::isdigit( name.front() ),
		std::string{"Invalid producer name: " + name}.c_str() );
}

std::shared_ptr<IBindable> IProducer::getBindable()
{
	THROW_RENDERER_EXCEPTION( "IProducer class cannot be a target of a Consumer!" );
}

std::shared_ptr<IRenderSurface> IProducer::getRenderSurface()
{
	THROW_RENDERER_EXCEPTION( "IProducer class cannot be a target of a Consumer!" );
}

const std::string& IProducer::getName() const noexcept
{
	return m_name;
}


}