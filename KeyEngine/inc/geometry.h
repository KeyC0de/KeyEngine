#pragma once

#include <optional>
#include "dynamic_vertex_buffer.h"
#include "triangle_mesh.h"


struct Geometry
{
	static TriangleMesh makeCube( std::optional<ver::VertexInputLayout> layout = {}, const float side = 1.0f / 2.0f );
	static TriangleMesh makeCubeIndependentFaces( ver::VertexInputLayout layout, const float side = 1.0f / 2.0f );
	static TriangleMesh makeCubeIndependentFacesTextured();
	static TriangleMesh makeSphereTesselated( std::optional<ver::VertexInputLayout> layout = std::nullopt, const unsigned nLateralDivs = 12, const unsigned nLongitudinalDivs = 24, const float radius = 1.0f );
	//	\function	makePlanarGridTextured	||	\date	2022/11/12 16:18
	//	\brief	nDivisionsX are length divisions
	static TriangleMesh makePlanarGrid( const int length = 2, const int width = 2, int nDivisionsX = 1, int nDivisionsY = 1 );
	static TriangleMesh makePlanarGridTextured( const int length = 2, const int width = 2, int nDivisionsX = 1, int nDivisionsY = 1 );
	//	\function	makePlanarGridTexturedFromHeighmap	||	\date	2022/11/21 0:06
	//	\brief	engine unit is cm, terrain has to be relatively large without consuming a vast amount of memory with indices and vertices, so we multiply its area by `terrainAreaUnitMultiplier`
	static TriangleMesh makePlanarGridTexturedFromHeighmap( const std::string &filename, const int normalizeAmount = 4, const int terrainAreaUnitMultiplier = 10, const int length = 2, const int width = 2, int nDivisionsX = 1, int nDivisionsY = 1 );
	static TriangleMesh makeCameraFrustum( const float width, const float height, const float nearZ, const float farZ );
	static TriangleMesh makeCameraWidget();
};