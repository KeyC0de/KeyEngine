#pragma once

#include <optional>
#include "dynamic_vertex_buffer.h"
#include "triangle_mesh.h"


struct Geometry
{
	static TriangleMesh makeCube( std::optional<ver::VertexInputLayout> layout = {} );
	static TriangleMesh makeCubeIndependentFaces( ver::VertexInputLayout layout );;
	static TriangleMesh makeCubeIndependentFacesTextured();

	static TriangleMesh makeTesselatedSphere( std::optional<ver::VertexInputLayout> layout = std::nullopt, const unsigned nLateralDivs = 12, const unsigned nLongitudinalDivs = 24 );

	static TriangleMesh makePlane( const int nDivisionsX = 1, const int nDivisionsY = 1 );
};