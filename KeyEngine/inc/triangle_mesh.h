#pragma once

#include <vector>
#include "dynamic_vertex_buffer.h"


struct TriangleMesh
{
	ver::Buffer m_vb;
	std::vector<unsigned> m_indices;

	TriangleMesh() = default;
	TriangleMesh( const ver::Buffer &vertices, const std::vector<unsigned> &indices );

	void transform( const DirectX::XMMATRIX &matrix );
	void setFlatShadedIndependentNormals() cond_noex;
};