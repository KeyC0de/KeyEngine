#include "algorithms.h"
#include <iostream>


namespace util
{

void regexSearch( const std::regex &pattern )
{
	int lineNo = 0;
	for ( std::string line; std::getline( std::cin, line ); )
	{
		++lineNo;
		std::smatch matches;
		if ( std::regex_search( line, matches, pattern ) )
		{
			std::cout << lineNo
				<< ": "
				<< matches[0]
				<< '\n';
		}
	}
}

}