#pragma once

#include <fstream>
#include <vector>
#include <string>
#include "key_timer.h"


class PerfLog final
{
	struct Entry final
	{
		float timeStamp;
		std::string label;

		Entry( float timeStamp, const std::string &lbl = "" );

		bool operator>( const Entry &rhs ) noexcept;
	};
	static inline PerfLog *m_pInstance;
private:
	std::string m_outFileName;
	KeyTimer<std::chrono::milliseconds> m_timer;
	std::vector<Entry> m_entries;
public:
	PerfLog( const std::string &outFileName = "perfTest.txt" ) noexcept;
	~PerfLog();
	PerfLog( const PerfLog &rhs ) = delete;
	PerfLog &operator=( const PerfLog &rhs ) = delete;

	void start( const std::string &lbl = "" ) noexcept;
	void lap( const std::string &lbl = "" ) noexcept;
private:
	void flushToFile();
};