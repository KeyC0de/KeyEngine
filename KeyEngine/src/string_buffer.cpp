#include "string_buffer.h"


StringBuffer::StringBuffer( const std::string &str,
	std::ostream &stream )
	:
	m_stream{stream}
{
	m_ss << str;
}

StringBuffer::StringBuffer( StringBuffer &&rhs ) noexcept
	:
	m_stream{rhs.m_stream}
{
	*( this ) = std::move( rhs );
}

StringBuffer& StringBuffer::operator=( StringBuffer &&rhs ) noexcept
{
	std::lock_guard<std::mutex> lg{s_mu};
	m_ss << rhs.m_ss.str();
	return *this;
}

StringBuffer::~StringBuffer() noexcept
{
	reset();
}

void StringBuffer::reset() noexcept
{
	m_ss.str( std::string{} );
	m_ss.clear();
}

std::string StringBuffer::get() const noexcept
{
	return m_ss.str();
}

void StringBuffer::print()
{
	std::lock_guard<std::mutex> lg{s_mu};
	m_stream << m_ss.str();
}