#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include "non_copyable.h"


///=============================================================
/// \class	StringBuffer
/// \author	KeyC0de
/// \date	2021/09/28 15:32
/// \brief	thread safe (given its allocator is thread safe) std::ostringstream - also guarantees consistent output
/// \brief	offers templated operator<< for appending into this stringstream object
/// \brief	but you must provide your own guards around string and stream objects that are shared among threads.
///=============================================================
class StringBuffer final
	: public NonCopyable
{
	static inline std::mutex s_mu;
	std::ostringstream m_ss;
	std::ostream &m_stream;

	void reset() noexcept;
public:
	StringBuffer( const std::string& str, std::ostream& stream = std::cout );
	StringBuffer( StringBuffer&& rhs ) noexcept;
	StringBuffer& operator=( StringBuffer&& rhs ) noexcept;
	~StringBuffer() noexcept;

	template<typename T>
	StringBuffer& operator<<( const T &str ) noexcept
	{
		m_ss << str;
		return *this;
	}
	std::string get() const noexcept;
	void print();
};