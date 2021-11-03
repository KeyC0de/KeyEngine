#include <csignal>


void installSigintHandler( int signum );
void installSigsegvHandler( int signum );
void installSigtermHandler( int signum );
void installSigillHandler( int signum );
void installSigabrtHandler( int signum );
void installSigfpeHandler( int signum );