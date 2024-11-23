#pragma once

#include <vector>


class PerlinNoise final
{
	std::vector<int> m_permutationVec;
 public:
	 /// \brief	generate a new permutation vector based on the value of seed
	PerlinNoise( unsigned int seed = 0 );

	/// \brief	get a noise value, for 2D images z can have any value
	double noise( double x, double y, double z );
private:
	double fade( const double t );
	double grad( const int hash, const double x, const double y, const double z );
};