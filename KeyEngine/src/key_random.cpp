#include "key_random.h"


void KeyRandom::init()
{
	m_device.seed( std::random_device{}() );
	m_started = true;
}

KeyRandom::operator bool() const noexcept
{
	return m_started;
}

float KeyRandom::getRandomFloatInRange( int start,
	int end )
{
	return start + (static_cast<float>( m_distr( m_device ) )
		/ static_cast<float>( std::numeric_limits<unsigned int>::max() )) * (end - start);
}

float KeyRandom::getRandomFloat()
{
	return static_cast<float>( m_distr( m_device ) ) / std::numeric_limits<float>::max();
}

double KeyRandom::getRandomDouble()
{
	return static_cast<double>(  m_distr( m_device ) ) / std::numeric_limits<double>::max();
}

double KeyRandom::getRandomDoubleInRange( int start,
	int end )
{
	return start + (static_cast<double>( m_distr( m_device ) )
		/ static_cast<double>( std::numeric_limits<unsigned int>::max() )) * (end - start);
}