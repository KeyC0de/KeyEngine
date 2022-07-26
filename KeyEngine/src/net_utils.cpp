#include <iostream>
#include <string>
#include "net_utils.h"


namespace net
{

void openWebpage( const std::string &address )
{
#if defined _MSC_VER || defined _WIN32 || defined _WIN64
	std::string str{"start " + address};
#elif defined __unix__ || defined __unix
	std::string str{"xdg-open " + address};
#elif defined __APPLE__ && defined __MACH__
	std::string str{"open " + address};
#endif
	std::system( str.c_str() );
}


}