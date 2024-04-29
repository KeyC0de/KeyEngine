#include "octree.h"


namespace dx = DirectX;

Octree::Octree( const dx::XMFLOAT3 &m_center,
	const dx::XMFLOAT3 &halfDim )
	:
	m_center( m_center ),
	m_half( halfDim ),
	m_data( nullptr )
{
	// initially there are no m_children
	for ( int i = 0; i < 8; ++i )
	{
		m_children[i] = nullptr;
	}
}

Octree::Octree( const Octree &rhs )
	:
	m_center( rhs.m_center ),
	m_half( rhs.m_half ),
	m_data( rhs.m_data )
{

}

Octree::~Octree()
{
	for ( int i = 0; i < 8; ++i )
	{
		delete m_children[i];
	}
}

int Octree::getOctantContainingPoint( const dx::XMFLOAT3 &point ) const
{
	int oct = 0;
	if ( point.x >= m_center.x )
	{
		oct |= 4;	// 0b 100
	}
	if ( point.y >= m_center.y )
	{
		oct |= 2;	// 0b 010
	}
	if ( point.z >= m_center.z )
	{
		oct |= 1;	// 0b 001
	}
	return oct;
}

bool Octree::isLeafNode() const
{
	/*
	// This is correct, but overkill
	for( int i = 0; i < 8; ++i )
	{
		if( m_children[i] != nullptr )
		{
			return false;
		}
	}
	return true;
	 */
	return m_children[0] == nullptr;
}

void Octree::insert( OctreeData *data )
{
	// If this node doesn't have a m_data node yet assigned
	// and it is a leaf, then we're done!
	if ( isLeafNode() )
	{
		if ( m_data == nullptr )
		{
			m_data = data;
			return;
		}
		else
		{
			// we're at a leaf, but there's already something here
			// we will split this node so that it has 8 children octants
			// and then insert `m_data` and `data` to children nodes

			// save the data for a later re-insert
			OctreeData *oldData = m_data;
			m_data = nullptr;

			// split the current node and create new empty trees for each child octant
			// compute bounding boxes for each child
			for ( int i = 0; i < 8; ++i )
			{
				dx::XMFLOAT3 newCenter = m_center;
				newCenter.x += m_half.x * ( i & 4 ? 0.5f : -0.5f );
				newCenter.y += m_half.y * ( i & 2 ? 0.5f : -0.5f );
				newCenter.z += m_half.z * ( i & 1 ? 0.5f : -0.5f );
				m_children[i] = new Octree{ newCenter, m_half * 0.5f };
			}

			// re-insert
			// (we wouldn't need to insert from the root, because we already
			// know it's guaranteed to be in this section of the tree)
			m_children[getOctantContainingPoint( oldData->getPosition() )]->insert( oldData );
			m_children[getOctantContainingPoint( data->getPosition() )]->insert( data );
		}
	}
	else
	{
		// We are at an interior node.
		// Insert recursively into the appropriate child octant until we reach a leaf
		int octant = getOctantContainingPoint( data->getPosition() );
		m_children[octant]->insert( data );
	}
}

void Octree::getEntitiesWithinBBox( const dx::XMFLOAT3 &requestedMin,
	const dx::XMFLOAT3 &requestedMax,
	std::vector<OctreeData*>& resultsOut )
{
	// if we're at a leaf node, check to see whether the point is inside the BBox
	if ( isLeafNode() )
	{
		if ( m_data != nullptr )
		{
			const dx::XMFLOAT3 p = m_data->getPosition();
			if ( p.x > requestedMax.x || p.y > requestedMax.y || p.z > requestedMax.z || p.x < requestedMin.x || p.y < requestedMin.y || p.z < requestedMin.z )
			{
				return;
			}
			resultsOut.push_back( m_data );
		}
	}
	else
	{
		// We're at an interior node of the tree. We will check to see if
		// the query bounding box lies outside the octants of this node.
		for ( int i = 0; i < 8; ++i )
		{
			// Compute the min/max corners of this child octant
			dx::XMFLOAT3 childMax = m_children[i]->m_center + m_children[i]->m_half;
			dx::XMFLOAT3 childMin = m_children[i]->m_center - m_children[i]->m_half;

			// check if child's {min,max} lie out of requested bounds
			if ( childMax.x < requestedMin.x
				|| childMax.y < requestedMin.y
				|| childMax.z < requestedMin.z
				|| childMin.x > requestedMax.x
				|| childMin.y > requestedMax.y
				|| childMin.z > requestedMax.z )
			{
				continue;
			}

			// we've determined that this child contains the requested bounding box
			m_children[i]->getEntitiesWithinBBox( requestedMin, requestedMax, resultsOut );
		}
	}
}