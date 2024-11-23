#pragma once

#include <cstddef>
#include <vector>
#include <DirectXMath.h>


using OctreeData = DirectX::XMFLOAT3;

///============================================================
/// \class	Octree
/// \link	https://github.com/brandonpelfrey/SimpleOctree
/// \author	brandonpelfrey
/// \brief	Octree
/// \brief	The tree has up to 8 children and can additionally store data (m_data),
/// \brief		Children follow a predictable pattern to make accesses simple.
/// \brief		- means less than 'm_center' in that dimension, + means greater than.
/// \brief	child:	0 1 2 3 4 5 6 7
/// \brief	x:      - - - - + + + +
/// \brief	y:      - - + + - - + +
/// \brief	z:      - + - + - + - +
///=============================================================
class Octree final
{
	DirectX::XMFLOAT3 m_center;	// the physical spatial center of this octree octant
	DirectX::XMFLOAT3 m_half;	// half the width,height,depth of this octant
	Octree *m_children[8];
	OctreeData *m_data;
private:
	// determine which octant of the tree contains point
	int getOctantContainingPoint( const DirectX::XMFLOAT3 &point ) const;
	bool isLeafNode() const;
public:
	Octree( const DirectX::XMFLOAT3 &center, const DirectX::XMFLOAT3 &halfDim );
	Octree( const Octree &rhs );
	~Octree();

	void insert( OctreeData *node );
	// recursively query the tree for Entities within a bounding box {bmin,bmax}
	void getEntitiesWithinBBox( const DirectX::XMFLOAT3 &bmin, const DirectX::XMFLOAT3 &bmax, std::vector<OctreeData*>& resultsOut );
};