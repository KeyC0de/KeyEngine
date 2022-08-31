#include "pass.h"
#include "consumer.h"


namespace ren
{

IPass::IPass( const std::string &name ) noexcept
	:
	m_name{name}
{

}

IPass::~IPass() noexcept
{
	pass_;
}

const std::string& IPass::getName() const noexcept
{
	return m_name;
}

void IPass::validate()
{
	for ( auto &consumer : m_consumers )
	{
		consumer->validateLinkage();
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

IConsumer& IPass::consumer( const std::string &name ) const
{
	for ( auto &consumer : m_consumers )
	{
		if ( consumer->getName() == name )
		{
			return *consumer;
		}
	}

	std::ostringstream oss;
	oss << "Consumer named ["
		<< name
		<< "] not found in pass:"
		<< getName();
	THROW_RENDERER_EXCEPTION( oss.str() );
}

IProducer& IPass::producer( const std::string &name ) const
{
	for ( auto &producer : m_producers )
	{ 
		if ( producer->getName() == name )
		{
			return *producer;
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
	// verify there are no name collisions with other consumers
	for ( auto &consumer : m_consumers )
	{
		if ( consumer->getName() == pConsumer->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Consumer name " + pConsumer->getName() + " collides with existing." );
		}
	}
	m_consumers.emplace_back( std::move( pConsumer ) );
}

void IPass::addProducer( std::unique_ptr<IProducer> pProducer )
{
	// verify there are no name collisions with other producers
	for ( auto &producer : m_producers )
	{
		if ( producer->getName() == pProducer->getName() )
		{
			THROW_RENDERER_EXCEPTION( "Producer name " + pProducer->getName() + " collides with existing." );
		}
	}
	m_producers.emplace_back( std::move( pProducer ) );
}

void IPass::setupConsumerTarget( const std::string &currentPassConsumerName,
	const std::string &targetPassName,
	const std::string &targetPassProducerName )
{
	auto &cons = consumer( currentPassConsumerName );
	cons.setPassAndProducerNames( targetPassName,
		targetPassProducerName );
}


}//ren