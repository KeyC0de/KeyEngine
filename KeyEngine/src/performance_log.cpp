#include <iomanip>
#include "performance_log.h"


PerfLog::PerfLog( const std::string &outFileName ) noexcept
	:
	m_outFileName{outFileName}
{
	m_entries.reserve( 100 );
}

PerfLog::~PerfLog()
{
	flushToFile();
}

PerfLog::Entry::Entry( float timeStamp,
	const std::string &lbl )
	:
	timeStamp( timeStamp ),
	label( lbl )
{

}

bool PerfLog::Entry::operator>( const Entry &rhs ) noexcept
{
	return timeStamp > rhs.timeStamp;
}

void PerfLog::flushToFile()
{
	std::ofstream file( "perf.txt" );
	file << std::fixed
		<< std::setprecision( 3 );
	for ( const auto &entry : m_entries )
	{
		using namespace std::string_literals;
		if ( entry.label.empty() )
		{
			file << entry.timeStamp
				<< " ms\n"s;
		}
		else
		{
			file << std::setw( 16 )
				<< std::left
				<< "["s + entry.label + "] "s
				<< std::setw( 8 )
				<< std::right
				<< entry.timeStamp
				<< " ms\n"s;
		}
	}
}

void PerfLog::start( const std::string &lbl ) noexcept
{
	m_timer.start();
	m_entries.emplace_back( 0.0f, lbl );
	m_timer.getTimeElapsed();
}

void PerfLog::lap( const std::string &lbl ) noexcept
{
	const float timeStamp = m_timer.lap();
	m_entries.emplace_back( timeStamp, lbl );
}