#pragma once

#include "geometry.h"
#include <DirectXMath.h>
#include <array>
#include "math_utils.h"
#include "assertions_console.h"


TriangleMesh Geometry::makeCube( std::optional<ver::VertexInputLayout> layout )
{
	using Type = ver::VertexInputLayout::ILEementType;

	if ( !layout )
	{
		layout = ver::VertexInputLayout{};
		layout->add( Type::Position3D );
	}

	constexpr float side = 1.0f / 2.0f;

	ver::VBuffer vertices{std::move( *layout ), 8u};
	vertices[0].element<Type::Position3D>() = {-side, -side, -side};
	vertices[1].element<Type::Position3D>() = {side, -side, -side};
	vertices[2].element<Type::Position3D>() = {-side, side, -side};
	vertices[3].element<Type::Position3D>() = {side, side, -side};
	vertices[4].element<Type::Position3D>() = {-side, -side, side};
	vertices[5].element<Type::Position3D>() = {side, -side, side};
	vertices[6].element<Type::Position3D>() = {-side, side, side};
	vertices[7].element<Type::Position3D>() = {side, side, side};

	return
	{
		vertices,
		{
			0, 2, 1, 2, 3, 1,
			1, 3, 5, 3, 7, 5,
			2, 6, 3, 3, 6, 7,
			4, 5, 7, 4, 7, 6,
			0, 4, 2, 2, 4, 6,
			0, 1, 4, 1, 5, 4
		}
	};
}

TriangleMesh Geometry::makeCubeIndependentFaces( ver::VertexInputLayout layout )
{
	using Type = ver::VertexInputLayout::ILEementType;

	constexpr float side = 1.0f / 2.0f;

	ver::VBuffer vertices{std::move( layout ), 24u};
	vertices[0].element<Type::Position3D>() = {-side, -side, -side};// 0 near side
	vertices[1].element<Type::Position3D>() = {side, -side, -side};// 1
	vertices[2].element<Type::Position3D>() = {-side, side, -side};// 2
	vertices[3].element<Type::Position3D>() = {side, side, -side};// 3
	vertices[4].element<Type::Position3D>() = {-side, -side, side};// 4 far side
	vertices[5].element<Type::Position3D>() = {side, -side, side};// 5
	vertices[6].element<Type::Position3D>() = {-side, side, side};// 6
	vertices[7].element<Type::Position3D>() = {side, side, side};// 7
	vertices[8].element<Type::Position3D>() = {-side, -side, -side};// 8 left side
	vertices[9].element<Type::Position3D>() = {-side, side, -side};// 9
	vertices[10].element<Type::Position3D>() = {-side, -side, side};// 10
	vertices[11].element<Type::Position3D>() = {-side, side, side};// 11
	vertices[12].element<Type::Position3D>() = {side, -side, -side};// 12 right side
	vertices[13].element<Type::Position3D>() = {side, side, -side};// 13
	vertices[14].element<Type::Position3D>() = {side, -side, side};// 14
	vertices[15].element<Type::Position3D>() = {side, side, side};// 15
	vertices[16].element<Type::Position3D>() = {-side, -side, -side};// 16 bottom side
	vertices[17].element<Type::Position3D>() = {side, -side, -side};// 17
	vertices[18].element<Type::Position3D>() = {-side, -side, side};// 18
	vertices[19].element<Type::Position3D>() = {side, -side, side};// 19
	vertices[20].element<Type::Position3D>() = {-side, side, -side};// 20 top side
	vertices[21].element<Type::Position3D>() = {side, side, -side};// 21
	vertices[22].element<Type::Position3D>() = {-side, side, side};// 22
	vertices[23].element<Type::Position3D>() = {side, side, side};// 23

	return
	{
		vertices,
		{
			0,	2,	1,	3,	1,	2,
			4,	5,	7,	4,	7,	6,
			8,	10, 9,	10, 11,	9,
			12, 13, 15, 12, 15, 14,
			16, 17, 18, 18, 17, 19,
			20, 23, 21, 20, 22, 23
		}
	};
}

TriangleMesh Geometry::makeCubeIndependentFacesTextured()
{
	using Type = ver::VertexInputLayout::ILEementType;

	auto itl = makeCubeIndependentFaces( std::move( ver::VertexInputLayout{}
		.add( Type::Position3D )
		.add( Type::Normal )
		.add( Type::Texture2D ) ) );

	auto &verts = itl.m_vb;

	verts[0].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[1].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[2].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[3].element<Type::Texture2D>() = {1.0f, 1.0f};
	verts[4].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[5].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[6].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[7].element<Type::Texture2D>() = {1.0f, 1.0f};
	verts[8].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[9].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[10].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[11].element<Type::Texture2D>() = {1.0f, 1.0f};
	verts[12].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[13].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[14].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[15].element<Type::Texture2D>() = {1.0f, 1.0f};
	verts[16].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[17].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[18].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[19].element<Type::Texture2D>() = {1.0f, 1.0f};
	verts[20].element<Type::Texture2D>() = {0.0f, 0.0f};
	verts[21].element<Type::Texture2D>() = {1.0f, 0.0f};
	verts[22].element<Type::Texture2D>() = {0.0f, 1.0f};
	verts[23].element<Type::Texture2D>() = {1.0f, 1.0f};

	return itl;
}

TriangleMesh Geometry::makeSphereTesselated( std::optional<ver::VertexInputLayout> layout,
	const unsigned nLateralDivs /* = 12 */,
	const unsigned nLongitudinalDivs /* = 24 */ )
{
	ASSERT( nLateralDivs >= 3, "Lateral divisions < 3 - too low!" );
	ASSERT( nLongitudinalDivs >= 3, "Longitudinal divisions < 3 - too low!" );

	if ( !layout )
	{
		layout = ver::VertexInputLayout{}.add( ver::VertexInputLayout::ILEementType::Position3D );
	}

	constexpr float radius = 1.0f;

	namespace dx = DirectX;
	const auto base = dx::XMVectorSet( 0.0f,
		0.0f,
		radius,
		0.0f );
	const float lattitudeAngle = util::PI / nLateralDivs;
	const float longitudeAngle = 2.0f * util::PI / nLongitudinalDivs;

	ver::VBuffer vb{std::move( *layout )};
	for ( unsigned lat = 1; lat < nLateralDivs; ++lat )
	{
		const auto latBase = dx::XMVector3Transform( base,
			dx::XMMatrixRotationX( lattitudeAngle * lat ) );
		for ( unsigned lon = 0; lon < nLongitudinalDivs; ++lon )
		{
			dx::XMFLOAT3 calculatedPos;
			auto v = dx::XMVector3Transform( latBase,
				dx::XMMatrixRotationZ( longitudeAngle * lon ) );
			dx::XMStoreFloat3( &calculatedPos,
				v );
			vb.emplaceVertex( calculatedPos );
		}
	}

	// add the cap vertices
	const auto iNorthPole = (unsigned)vb.getVertexCount();
	{
		dx::XMFLOAT3 northPos;
		dx::XMStoreFloat3( &northPos,
			base );
		vb.emplaceVertex( northPos );
	}

	const auto iSouthPole = (unsigned)vb.getVertexCount();
	{
		dx::XMFLOAT3 southPos;
		dx::XMStoreFloat3( &southPos,
			dx::XMVectorNegate( base ) );
		vb.emplaceVertex( southPos );
	}

	const auto calcIdx = [nLateralDivs, nLongitudinalDivs]
		( unsigned lat, unsigned lon )
	{
		return lat * nLongitudinalDivs + lon;
	};

	std::vector<unsigned> indices;
	for ( unsigned lat = 0; lat < nLateralDivs - 2; ++lat )
	{
		for ( unsigned lon = 0; lon < nLongitudinalDivs - 1; ++lon )
		{
			indices.push_back( calcIdx( lat, lon ) );
			indices.push_back( calcIdx( lat + 1, lon ) );
			indices.push_back( calcIdx( lat, lon + 1 ) );
			indices.push_back( calcIdx( lat, lon + 1 ) );
			indices.push_back( calcIdx( lat + 1, lon ) );
			indices.push_back( calcIdx( lat + 1, lon + 1 ) );
		}
		// wrap band
		indices.push_back( calcIdx( lat, nLongitudinalDivs - 1 ) );
		indices.push_back( calcIdx( lat + 1, nLongitudinalDivs - 1 ) );
		indices.push_back( calcIdx( lat, 0 ) );
		indices.push_back( calcIdx( lat, 0 ) );
		indices.push_back( calcIdx( lat + 1, nLongitudinalDivs - 1 ) );
		indices.push_back( calcIdx( lat + 1, 0 ) );
	}

	// cap fans
	for ( unsigned lon = 0; lon < nLongitudinalDivs - 1; ++lon )
	{
		// north
		indices.push_back( iNorthPole );
		indices.push_back( calcIdx( 0, lon ) );
		indices.push_back( calcIdx( 0, lon + 1 ) );
		// south
		indices.push_back( calcIdx( nLateralDivs - 2, lon + 1 ) );
		indices.push_back( calcIdx( nLateralDivs - 2, lon ) );
		indices.push_back( iSouthPole );
	}

	// wrap triangles
	// north
	indices.push_back( iNorthPole );
	indices.push_back( calcIdx( 0,
		nLongitudinalDivs - 1 ) );
	indices.push_back( calcIdx( 0,
		0 ) );
	// south
	indices.push_back( calcIdx( nLateralDivs - 2,
		0 ) );
	indices.push_back( calcIdx( nLateralDivs - 2,
		nLongitudinalDivs - 1 ) );
	indices.push_back( iSouthPole );

	return {vb, indices};
}

TriangleMesh Geometry::makePlanarGrid( const float length /*= 2.0f*/,
	const float width /*= 2.0f*/,
	int nDivisionsX /*= 1*/,
	int nDivisionsY /*= 1*/ )
{
	ASSERT( nDivisionsX >= 1, "X Divisions can't be less than 1!" );
	ASSERT( nDivisionsY >= 1, "Y Divisions can't be less than 1!" );
	ASSERT( nDivisionsX >= 1, "Longitudinal divisions have to be >= 1!" );
	ASSERT( nDivisionsY >= 1, "Laterial divisions have to be >= 1!" );

	if ( length > 2.0f && width > 2.0f && nDivisionsX == 1 && nDivisionsY == 1 )
	{
		nDivisionsX = 2;
		nDivisionsY = 2;
	}

	const int nVerticesX = nDivisionsX + 1;
	const int nVerticesY = nDivisionsY + 1;

	ver::VertexInputLayout layout;
	layout.add( ver::VertexInputLayout::Position3D );

	ver::VBuffer vb{std::move( layout )};
	{
		const float sideX = length / 2.0f;
		const float sideY = width / 2.0f;
		const float nXDivisions = length / float( nDivisionsX );
		const float nYDivisions = width / float( nDivisionsY );

		for ( int y = 0; y < nVerticesY; ++y )
		{
			const float yPos = float( y ) * nYDivisions - sideY;
			for ( int x = 0; x < nVerticesX; ++x )
			{
				const float xPos = float( x ) * nXDivisions - sideX;
				vb.emplaceVertex( DirectX::XMFLOAT3{xPos, yPos, 0.0f} );
			}
		}
	}

	std::vector<unsigned> indices;
	indices.reserve( util::square( nDivisionsX * nDivisionsY ) * 6 );
	const auto calcCoords = [nVerticesX]( size_t x, size_t y )
	{
		return (unsigned)( y * nVerticesX + x );
	};

	for ( size_t y = 0; y < nDivisionsY; ++y )
	{
		for ( size_t x = 0; x < nDivisionsX; ++x )
		{
			const std::array<unsigned, 4> indexArray =
			{
				calcCoords( x, y ),
				calcCoords( x + 1, y ),
				calcCoords( x, y + 1 ),
				calcCoords( x + 1, y + 1 )
			};
			indices.push_back( indexArray[0] );
			indices.push_back( indexArray[2] );
			indices.push_back( indexArray[1] );
			indices.push_back( indexArray[1] );
			indices.push_back( indexArray[2] );
			indices.push_back( indexArray[3] );
		}
	}

	return {vb, indices};
}

TriangleMesh Geometry::makePlanarGridTextured( const float length /*= 2.0f*/,
	const float width /*= 2.0f*/,
	int nDivisionsX /*= 1*/,
	int nDivisionsY /*= 1*/)
{
	ASSERT( length >= 2, "Length has to be greater than 1!" );
	ASSERT( width >= 2, "Width has to be greater than 1!" );
	ASSERT( nDivisionsX >= 1, "Longitudinal divisions have to be >= 1!" );
	ASSERT( nDivisionsY >= 1, "Laterial divisions have to be >= 1!" );

	if ( length > 2.0f && width > 2.0f && nDivisionsX == 1 && nDivisionsY == 1 )
	{
		nDivisionsX = 2;
		nDivisionsY = 2;
	}

	const int nVerticesX = nDivisionsX + 1;
	const int nVerticesY = nDivisionsY + 1;

	// eg. Edge: (*=vertex) with nVerticesX=2: nDivisionsX=1 *-------------------*-------------------	#TODO: connect final edge vertex
	////////////////////////////////////////////////////////////////////////////////////////////////////

	ver::VertexInputLayout layout;
	layout.add( ver::VertexInputLayout::Position3D );
	layout.add( ver::VertexInputLayout::Normal );
	layout.add( ver::VertexInputLayout::Texture2D );

	ver::VBuffer vb{std::move( layout )};
	{
		const float halfLength = length / 2.0f;
		const float halfWidth = width / 2.0f;
		const float segmentLength = length / float( nDivisionsX );
		const float segmentWidth = width / float( nDivisionsY );
		const float nUDivisions = 1.0f / float( nDivisionsX );
		const float nVDivisions = 1.0f / float( nDivisionsY );

		for ( int y = 0; y < nVerticesY; ++y )
		{
			const float yPos = float( y ) * segmentWidth - halfWidth;
			const float vPos = 1.0f - float( y ) * nVDivisions;
			for ( int x = 0; x < nVerticesX; ++x )
			{
				const float xPos = float( x ) * segmentLength - halfLength;
				const float uPos = float( x ) * nUDivisions;
				vb.emplaceVertex( DirectX::XMFLOAT3{xPos, yPos, 0.0f},
					DirectX::XMFLOAT3{0.0f, 0.0f, -1.0f},
					DirectX::XMFLOAT2{uPos, vPos} );
			}
		}
	}

	std::vector<unsigned> indices;
	indices.reserve( util::square( nDivisionsX * nDivisionsY ) * 6 );
	const auto calcCoords = [nVerticesX]( const size_t x, const size_t y )
	{
		return (unsigned)( y * nVerticesX + x );
	};

	for ( size_t y = 0; y < nDivisionsY; ++y )
	{
		for ( size_t x = 0; x < nDivisionsX; ++x )
		{
			const std::array<unsigned, 4> indexArray =
			{
				calcCoords( x, y ),
				calcCoords( x + 1, y ),
				calcCoords( x, y + 1 ),
				calcCoords( x + 1, y + 1 )
			};
			// each quad needs 6 indices
			indices.push_back( indexArray[0] );
			indices.push_back( indexArray[2] );
			indices.push_back( indexArray[1] );
			indices.push_back( indexArray[1] );
			indices.push_back( indexArray[2] );
			indices.push_back( indexArray[3] );
		}
	}

	return {vb, indices};
}