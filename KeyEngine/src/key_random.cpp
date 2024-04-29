#include "key_random.h"


KeyRandom::KeyRandom( bool started /* = true */ )
	:
	m_bStarted{started}
{
	if ( started )
	{
		start();
	}
}

void KeyRandom::start() noexcept
{
	m_device.seed( std::random_device{}() );
}

KeyRandom::operator bool() noexcept
{
	return m_bStarted;
}

int KeyRandom::rand()
{
	return static_cast<int>( m_distr( m_device ) );
}

int KeyRandom::getRandomInt()
{
	return static_cast<int>( m_distr( m_device ) ) / std::numeric_limits<int>::max();
}

float KeyRandom::getRandomFloat()
{
	return static_cast<float>( m_distr( m_device ) ) / std::numeric_limits<float>::max();
}

double KeyRandom::getRandomDouble()
{
	return static_cast<double>(  m_distr( m_device ) ) / std::numeric_limits<double>::max();
}

int KeyRandom::getRandomIntInRange( const int start,
	const int end )
{
	std::uniform_int_distribution<int> intDistr{start, end};
	return intDistr( m_device );
}

float KeyRandom::getRandomFloatInRange( const int start,
	const int end )
{
	return start
		+ ( static_cast<float>( m_distr( m_device ) ) / static_cast<float>( std::numeric_limits<unsigned int>::max() ) )
		* ( end - start );
}

double KeyRandom::getRandomDoubleInRange( const int start,
	const int end )
{
	return start
		+ ( static_cast<double>( m_distr( m_device ) ) / static_cast<double>( std::numeric_limits<unsigned int>::max() ) )
		* ( end - start );
}