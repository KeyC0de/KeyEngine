#pragma once

#include <optional>
#include <DirectXMath.h>
#include "dynamic_vertex_buffer.h"
#include "triangle_mesh.h"


struct GeometryCube
{
	static TriangleMesh make( std::optional<ver::VertexLayout> layout = {} );
	static TriangleMesh makeIndependentFaces( ver::VertexLayout layout );;
	static TriangleMesh makeIndependentFacesTextured();
};

struct GeometrySphere
{
	// make tesselated sphere
	static TriangleMesh make( std::optional<ver::VertexLayout> layout = std::nullopt,
		unsigned nLateralDivs = 12, unsigned nLongitudinalDivs = 24 );
};

struct GeometryPlane
{
	static TriangleMesh make( int nDivisionsX = 1, int nDivisionsY = 1 );
};