#pragma once

#include <vector>
#include "dynamic_vertex_buffer.h"


struct TriangleMesh final
{
	ver::VBuffer m_vb;
	std::vector<unsigned> m_indices;

	TriangleMesh() = default;
	TriangleMesh( const ver::VBuffer &vertices, const std::vector<unsigned> &indices, const bool bMultimesh = false );

	void transform( const DirectX::XMMATRIX &matrix );
	void setFlatShadedIndependentNormals() cond_noex;
};