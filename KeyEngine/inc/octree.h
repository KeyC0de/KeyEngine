#pragma once

#include <cstddef>
#include <vector>
#include "vec3.h"


class OctreeData;

//============================================================
//	\class	Octree
//	\link	https://github.com/brandonpelfrey/SimpleOctree
//
//	\author	brandonpelfrey
//
//	\brief	Octree
//		The tree has up to 8 children and can additionally store data (m_data),
//			Children follow a predictable pattern to make accesses simple.
//			- means less than 'm_center' in that dimension, + means greater than.
//		child:	0 1 2 3 4 5 6 7
//		x:      - - - - + + + +
//		y:      - - + + - - + +
//		z:      - + - + - + - +
//=============================================================
class Octree final
{
	Vec3 m_center;	// the physical spatial center of this octree octant
	Vec3 m_half;	// half the width,height,depth of this octant
	Octree *m_children[8];
	class OctreeData *m_data;

private:
	// determine which octant of the tree contains point
	int getOctantContainingPoint( const Vec3 &point ) const;
	bool isLeafNode() const;
public:
	Octree( const Vec3 &center, const Vec3 &halfDim );
	Octree( const Octree &rhs );
	~Octree();

	void insert( class OctreeData *node );
	// recursively query the tree for Entities within a bounding box {bmin,bmax}
	void getEntitiesWithinBBox( const Vec3 &bmin, const Vec3 &bmax,
		std::vector<class OctreeData*>& resultsOut );
};