#include "perlin_noise.h"
#include <numeric>
#include <ctime>
#include <random>
#include <algorithm>
#include "math_utils.h"


PerlinNoise::PerlinNoise( unsigned int seed /*= 0*/ )
{
	const auto t = util::timeTtoSeconds( time( nullptr ) );
	srand( t );

	seed = (seed == 0) ? t : seed;

	m_permutationVec.resize( 256 );

	// fill p with values from 0 to 255
	std::iota( m_permutationVec.begin(), m_permutationVec.end(), 0 );

	// initialize a random engine with seed
	std::default_random_engine engine( seed );

	// shuffle using the above random engine
	std::shuffle( m_permutationVec.begin(), m_permutationVec.end(), engine );

	// duplicate the permutation vector
	m_permutationVec.insert( m_permutationVec.end(), m_permutationVec.begin(), m_permutationVec.end() );
}

double PerlinNoise::noise( double x,
	double y,
	double z )
{
	// Find the unit cube that contains the point
	int X = (int) floor( x ) & 255;
	int Y = (int) floor( y ) & 255;
	int Z = (int) floor( z ) & 255;

	// Find relative x, y, z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = m_permutationVec[X] + Y;
	int AA = m_permutationVec[A] + Z;
	int AB = m_permutationVec[A + 1] + Z;
	int B = m_permutationVec[X + 1] + Y;
	int BA = m_permutationVec[B] + Z;
	int BB = m_permutationVec[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = util::lerp(
		util::lerp(
				util::lerp(grad(m_permutationVec[AA], x, y, z), grad(m_permutationVec[BA], x - 1, y, z), u),
				util::lerp(grad(m_permutationVec[AB], x, y - 1, z), grad(m_permutationVec[BB], x - 1, y - 1, z), u),
			v),
		util::lerp(
				util::lerp(grad(m_permutationVec[AA + 1], x, y, z - 1), grad(m_permutationVec[BA + 1], x - 1, y, z - 1), u),
				util::lerp(grad(m_permutationVec[AB + 1], x, y - 1, z - 1),	grad(m_permutationVec[BB + 1], x - 1, y - 1, z - 1), u),
				v),
		w);
	return ( res + 1.0 ) / 2.0;
}

double PerlinNoise::fade( const double t )
{
	return t * t * t * ( t * ( t * 6 - 15 ) + 10 );
}

double PerlinNoise::grad( const int hash,
	const double x,
	const double y,
	const double z )
{
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ?
		x :
		y;
	double v = h < 4 ?
		y :
		h == 12 || h == 14 ? x : z;
	return ( (h & 1) == 0 ? u : -u ) + ( (h & 2) == 0 ? v : -v );
}