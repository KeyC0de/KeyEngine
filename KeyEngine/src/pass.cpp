#include "pass.h"
#include "consumer.h"


namespace ren
{

IPass::IPass( const std::string &name ) noexcept
	:
	m_name{name}
{

}

IPass::~IPass()
{
	pass_;
}

const std::string &IPass::getName() const noexcept
{
	return m_name;
}

void IPass::validate()
{
	for ( auto& cons : m_consumers )
	{
		cons->validateLinkage();
	}
}

const std::vector<std::unique_ptr<IConsumer>>& IPass::getConsumers() const
{
	return m_consumers;
}

const std::vector<std::unique_ptr<IProducer>>& IPass::getProducers() const
{
	return m_producers;
}

IConsumer& IPass::getConsumer( const std::string &name ) const
{
	for ( auto& cons : m_consumers )
	{
		if ( cons->getName() == name )
		{
			return *cons;
		}
	}

	std::ostringstream oss;
	oss << "Consumer named ["
		<< name
		<< "] not found in pass:"
		<< getName();
	THROW_RENDERER_EXCEPTION( oss.str() );
}

IProducer& IPass::getProducer( const std::string &name ) const
{
	for ( auto& prod : m_producers )
	{
		if ( prod->getName() == name )
		{
			return *prod;
		}
	}

	std::ostringstream oss;
	oss << "Producer named ["
		<< name
		<< "] not found in pass: "
		<< getName();
	THROW_RENDERER_EXCEPTION( oss.str() );
}

void IPass::addConsumer( std::unique_ptr<IConsumer> pConsumer )
{
	// verify there are no consumer name collisions
	for ( auto& cons : m_consumers )
	{
		if ( cons->getName() == pConsumer->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Consumer name " + pConsumer->getName()
				+ " collides with existing." );
		}
	}
	m_consumers.push_back( std::move( pConsumer ) );
}

void IPass::addProducer( std::unique_ptr<IProducer> pProducer )
{
	// verify there are no producer name collisions
	for ( auto& prod : m_producers )
	{
		if ( prod->getName() == pProducer->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Producer name " + pProducer->getName()
				+ " collides with existing." );
		}
	}
	m_producers.push_back( std::move( pProducer ) );
}

void IPass::setupConsumerTarget( const std::string &consumerName,
	const std::string &targetPassName,
	const std::string &targetPassProducerName )
{
	auto& cons = getConsumer( consumerName );
	cons.setPassAndProducerNames( targetPassName,
		targetPassProducerName );
}


}//ren