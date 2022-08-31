#include <csignal>


void installSigintHandler( const int signum );
void installSigsegvHandler( const int signum );
void installSigtermHandler( const int signum );
void installSigillHandler( const int signum );
void installSigabrtHandler( const int signum );
void installSigfpeHandler( const int signum );