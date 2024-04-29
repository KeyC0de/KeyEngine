#include "linker.h"
#include <algorithm>
#include <cctype>
#include "assertions_console.h"


namespace ren
{

ILinker::ILinker( const std::string &name )
	:
	m_name{name}
{
	ASSERT( !name.empty(), "Linker name is null!" );

	const bool bNameValid = std::all_of( name.begin(), name.end(),
		[]( char c )
		{
			return std::isalnum( c ) || c == '_';
		} );

	ASSERT( bNameValid && !std::isdigit( name.front() ),
		std::string{"Invalid linker name: " + name}.c_str() );
}

std::shared_ptr<IBindable> ILinker::getBindable()
{
	THROW_RENDERER_EXCEPTION( "ILinker class cannot be a target of a Binder!" );
}

std::shared_ptr<IRenderSurface> ILinker::getRenderSurface()
{
	ASSERT( false, "Should never happen!" );
	THROW_RENDERER_EXCEPTION( "ILinker class cannot be a target of a Binder!" );
}

const std::string& ILinker::getName() const noexcept
{
	return m_name;
}


}