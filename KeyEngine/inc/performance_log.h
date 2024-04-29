#pragma once

#include <fstream>
#include <vector>
#include <string>
#include "key_timer.h"
#include "non_copyable.h"


class PerfLog final
	: public NonCopyable
{
	struct Entry final
	{
		float timeStamp;
		std::string label;

		Entry( float timeStamp, const std::string &lbl = "" );

		bool operator>( const Entry &rhs ) noexcept;
	};
private:
	std::string m_outFileName;
	KeyTimer<std::chrono::milliseconds> m_timer;
	std::vector<Entry> m_entries;
public:
	PerfLog( const std::string &outFileName = "perfTest.txt" ) noexcept;
	~PerfLog();

	void start( const std::string &lbl = "" ) noexcept;
	void lap( const std::string &lbl = "" ) noexcept;
private:
	void flushToFile();
};