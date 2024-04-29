#pragma once

#include "geometry.h"
#include <DirectXMath.h>
#include <array>
#include "math_utils.h"
#include "assertions_console.h"
#include "bmp_loader.h"


namespace dx = DirectX;

TriangleMesh Geometry::makeCube( std::optional<ver::VertexInputLayout> layout,
	const float side/* = 1.0f / 2.0f*/ )
{
	using Type = ver::VertexInputLayout::ILEementType;

	if ( !layout )
	{
		layout = ver::VertexInputLayout{};
		layout->add( Type::Position3D );
	}

	ver::VBuffer vb{std::move( *layout ), 8u};
	vb[0].getElement<Type::Position3D>() = {-side, -side, -side};
	vb[1].getElement<Type::Position3D>() = {side, -side, -side};
	vb[2].getElement<Type::Position3D>() = {-side, side, -side};
	vb[3].getElement<Type::Position3D>() = {side, side, -side};
	vb[4].getElement<Type::Position3D>() = {-side, -side, side};
	vb[5].getElement<Type::Position3D>() = {side, -side, side};
	vb[6].getElement<Type::Position3D>() = {-side, side, side};
	vb[7].getElement<Type::Position3D>() = {side, side, side};

	return
	{
		vb,
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

TriangleMesh Geometry::makeCubeIndependentFaces( ver::VertexInputLayout layout,
	const float side/* = 1.0f / 2.0f*/ )
{
	using Type = ver::VertexInputLayout::ILEementType;

	ver::VBuffer vb{std::move( layout ), 24u};
	vb[0].getElement<Type::Position3D>() = {-side, -side, -side};// 0 near side
	vb[1].getElement<Type::Position3D>() = {side, -side, -side};// 1
	vb[2].getElement<Type::Position3D>() = {-side, side, -side};// 2
	vb[3].getElement<Type::Position3D>() = {side, side, -side};// 3
	vb[4].getElement<Type::Position3D>() = {-side, -side, side};// 4 far side
	vb[5].getElement<Type::Position3D>() = {side, -side, side};// 5
	vb[6].getElement<Type::Position3D>() = {-side, side, side};// 6
	vb[7].getElement<Type::Position3D>() = {side, side, side};// 7
	vb[8].getElement<Type::Position3D>() = {-side, -side, -side};// 8 left side
	vb[9].getElement<Type::Position3D>() = {-side, side, -side};// 9
	vb[10].getElement<Type::Position3D>() = {-side, -side, side};// 10
	vb[11].getElement<Type::Position3D>() = {-side, side, side};// 11
	vb[12].getElement<Type::Position3D>() = {side, -side, -side};// 12 right side
	vb[13].getElement<Type::Position3D>() = {side, side, -side};// 13
	vb[14].getElement<Type::Position3D>() = {side, -side, side};// 14
	vb[15].getElement<Type::Position3D>() = {side, side, side};// 15
	vb[16].getElement<Type::Position3D>() = {-side, -side, -side};// 16 bottom side
	vb[17].getElement<Type::Position3D>() = {side, -side, -side};// 17
	vb[18].getElement<Type::Position3D>() = {-side, -side, side};// 18
	vb[19].getElement<Type::Position3D>() = {side, -side, side};// 19
	vb[20].getElement<Type::Position3D>() = {-side, side, -side};// 20 top side
	vb[21].getElement<Type::Position3D>() = {side, side, -side};// 21
	vb[22].getElement<Type::Position3D>() = {-side, side, side};// 22
	vb[23].getElement<Type::Position3D>() = {side, side, side};// 23

	return
	{
		vb,
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

	verts[0].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[1].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[2].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[3].getElement<Type::Texture2D>() = {1.0f, 1.0f};
	verts[4].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[5].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[6].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[7].getElement<Type::Texture2D>() = {1.0f, 1.0f};
	verts[8].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[9].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[10].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[11].getElement<Type::Texture2D>() = {1.0f, 1.0f};
	verts[12].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[13].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[14].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[15].getElement<Type::Texture2D>() = {1.0f, 1.0f};
	verts[16].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[17].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[18].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[19].getElement<Type::Texture2D>() = {1.0f, 1.0f};
	verts[20].getElement<Type::Texture2D>() = {0.0f, 0.0f};
	verts[21].getElement<Type::Texture2D>() = {1.0f, 0.0f};
	verts[22].getElement<Type::Texture2D>() = {0.0f, 1.0f};
	verts[23].getElement<Type::Texture2D>() = {1.0f, 1.0f};

	return itl;
}

TriangleMesh Geometry::makeSphereTesselated( std::optional<ver::VertexInputLayout> layout,
	const unsigned nLateralDivs /*= 12*/,
	const unsigned nLongitudinalDivs /*= 24*/,
	const float radius /*= 1.0f*/ )
{
	ASSERT( nLateralDivs >= 3, "Lateral divisions < 3 - too low!" );
	ASSERT( nLongitudinalDivs >= 3, "Longitudinal divisions < 3 - too low!" );

	if ( !layout )
	{
		layout = ver::VertexInputLayout{}.add( ver::VertexInputLayout::ILEementType::Position3D );
	}

	namespace dx = DirectX;
	const auto base = dx::XMVectorSet( 0.0f, 0.0f, radius, 0.0f );
	const float lattitudeAngle = util::PI / nLateralDivs;
	const float longitudeAngle = 2.0f * util::PI / nLongitudinalDivs;

	ver::VBuffer vb{std::move( *layout )};
	for ( unsigned lat = 1; lat < nLateralDivs; ++lat )
	{
		const auto latBase = dx::XMVector3Transform( base, dx::XMMatrixRotationX( lattitudeAngle * lat ) );
		for ( unsigned lon = 0; lon < nLongitudinalDivs; ++lon )
		{
			dx::XMFLOAT3 calculatedPos{};
			auto v = dx::XMVector3Transform( latBase, dx::XMMatrixRotationZ( longitudeAngle * lon ) );
			dx::XMStoreFloat3( &calculatedPos, v );
			vb.emplaceVertex( calculatedPos );
		}
	}

	// add the cap vb
	const auto iNorthPole = (unsigned)vb.getVertexCount();
	{
		dx::XMFLOAT3 northPos{};
		dx::XMStoreFloat3( &northPos, base );
		vb.emplaceVertex( northPos );
	}

	const auto iSouthPole = (unsigned)vb.getVertexCount();
	{
		dx::XMFLOAT3 southPos{};
		dx::XMStoreFloat3( &southPos, dx::XMVectorNegate( base ) );
		vb.emplaceVertex( southPos );
	}

	const auto calcIdx = [nLateralDivs, nLongitudinalDivs] ( unsigned lat, unsigned lon )
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
	indices.push_back( calcIdx( 0, nLongitudinalDivs - 1 ) );
	indices.push_back( calcIdx( 0, 0 ) );
	// south
	indices.push_back( calcIdx( nLateralDivs - 2, 0 ) );
	indices.push_back( calcIdx( nLateralDivs - 2, nLongitudinalDivs - 1 ) );
	indices.push_back( iSouthPole );

	return {vb, indices};
}

TriangleMesh Geometry::makePlanarGrid( const int length /*= 2*/,
	const int width /*= 2*/,
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
	indices.reserve( nDivisionsX * nDivisionsY * 6 );
	const auto calcCoords = [nVerticesX] ( size_t x, size_t y )
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

TriangleMesh Geometry::makePlanarGridTextured( const int length /*= 2*/,
	const int width /*= 2*/,
	int nDivisionsX /*= 1*/,
	int nDivisionsY /*= 1*/ )
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
		const float du = 1.0f / float( nDivisionsX );	// u & v range /in [0,1]
		const float dv = 1.0f / float( nDivisionsY );

		for ( int y = 0; y < nVerticesY; ++y )
		{
			const float yPos = float( y ) * segmentWidth - halfWidth;
			const float vPos = 1.0f - float( y ) * dv;
			for ( int x = 0; x < nVerticesX; ++x )
			{
				const float xPos = float( x ) * segmentLength - halfLength;
				const float uPos = float( x ) * du;
				vb.emplaceVertex( DirectX::XMFLOAT3{xPos, yPos, 0.0f}, DirectX::XMFLOAT3{0.0f, 0.0f, -1.0f}, DirectX::XMFLOAT2{uPos, vPos} );
			}
		}
	}

	std::vector<unsigned> indices;
	indices.reserve( nDivisionsX * nDivisionsY * 6 );
	const auto calcCoords = [nVerticesX] ( const size_t x, const size_t y )
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

TriangleMesh Geometry::makePlanarGridTexturedFromHeighmap( const std::string &filename,
	const int normalizeAmount /*= 4*/,
	const int terrainAreaUnitMultiplier /*= 10*/,
	const float length /*= 2.0f*/,
	const float width /*= 2.0f*/,
	int nDivisionsX /*= 1*/,
	int nDivisionsY /*= 1*/ )
{
	ASSERT( length >= 2, "Length has to be greater than 1!" );
	ASSERT( width >= 2, "Width has to be greater than 1!" );
	ASSERT( nDivisionsX >= 1, "Longitudinal divisions have to be >= 1!" );
	ASSERT( nDivisionsY >= 1, "Laterial divisions have to be >= 1!" );
	ASSERT( normalizeAmount >= 0, "Normalization amount must be greater or equal to 0(ie no normalization of heightmap values)!" );

	if ( length > 2.0f && width > 2.0f && nDivisionsX == 1 && nDivisionsY == 1 )
	{
		nDivisionsX = 2;
		nDivisionsY = 2;
	}

	const int nVerticesX = nDivisionsX + 1;
	const int nVerticesY = nDivisionsY + 1;

	// setup the heightmap
	BmpLoader bmp{filename};
	const size_t imageWidth = bmp.getWidth();
	const size_t imageHeight = bmp.getHeight();
	const size_t numImageElements = imageWidth * imageHeight;
	BmpLoader::ImageData *img = new BmpLoader::ImageData[numImageElements];

	ASSERT( bmp.getBitCount() == 24, "Not made to handle non-24bit width heightmaps!" );	// #TODO: rework this into a try catch exception where you release `img`

	bmp.readData( img );
	if ( normalizeAmount )
	{
		bmp.normalizeHeightmap( img, normalizeAmount );
	}

	const int imageDx = 3 * ( std::max( util::floor( (float) imageWidth / nVerticesX ), 1 ) );	// * 3 because the height value is replicated 3 times in this 24bit heightmap

	// setup the Input Layout
	ver::VertexInputLayout layout;
	layout.add( ver::VertexInputLayout::Position3D );
	layout.add( ver::VertexInputLayout::Normal );
	layout.add( ver::VertexInputLayout::Texture2D );

	// setup the vb
	ver::VBuffer vb{std::move( layout )};
	{
		const float halfLength = length / 2.0f;
		const float halfWidth = width / 2.0f;
		const float segmentLength = length / float( nDivisionsX );
		const float segmentWidth = width / float( nDivisionsY );
		const float du = 1.0f / float( nDivisionsX );	// u & v range /in [0,1]
		const float dv = 1.0f / float( nDivisionsY );

		for ( int y = 0; y < nVerticesY; ++y )
		{
			const float yPos = float( y ) * segmentWidth - halfWidth;
			const float vPos = 1.0f - float( y ) * dv;
			for ( int x = 0; x < nVerticesX; ++x )
			{
				const float xPos = float( x ) * segmentLength - halfLength;
				const float uPos = float( x ) * du;

				const float heightValue = static_cast<uint8_t>(img[y * imageWidth + x * imageDx]._24bit.b);

				// add the heightValue to the z coordinate (and not to the y - height) because the grid is not created on the x-y plane - we will pass an initialRotation to it upon creation
				vb.emplaceVertex( DirectX::XMFLOAT3{xPos * terrainAreaUnitMultiplier, yPos * terrainAreaUnitMultiplier, heightValue}, DirectX::XMFLOAT3{0.0f, 0.0f, -1.0f}, DirectX::XMFLOAT2{uPos, vPos} );
			}
		}
	}

	// heightmap has been read so delete the memory consumed by its associated image data array
	delete[] img;

	// setup the indices
	std::vector<unsigned> indices;
	indices.reserve( nDivisionsX * nDivisionsY * 6 );
	const auto calcCoords = [nVerticesX] ( const size_t x, const size_t y )
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

TriangleMesh Geometry::makeCameraWidget()
{
	ver::VertexInputLayout vertexLayout;
	vertexLayout.add( ver::VertexInputLayout::Position3D );

	ver::VBuffer vb{std::move( vertexLayout )};
	const float x = 4.0f / 3.0f * 0.75f;
	const float y = 1.0f * 0.75f;
	const float z = -2.0f;
	const float xHalf = x * 0.5f;
	const float tSpace = y * 0.2f;
	vb.emplaceVertex( dx::XMFLOAT3{-x, y, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{x, y, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{x, -y, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{-x, -y, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{0.0f, 0.0f, z} );
	vb.emplaceVertex( dx::XMFLOAT3{-xHalf, y + tSpace, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{xHalf, y + tSpace, 0.0f} );
	vb.emplaceVertex( dx::XMFLOAT3{0.0f, y + tSpace + xHalf, 0.0f} );

	std::vector<unsigned> indices;
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 1 );
	indices.push_back( 2 );
	indices.push_back( 2 );
	indices.push_back( 3 );
	indices.push_back( 3 );
	indices.push_back( 0 );
	indices.push_back( 0 );
	indices.push_back( 4 );
	indices.push_back( 1 );
	indices.push_back( 4 );
	indices.push_back( 2 );
	indices.push_back( 4 );
	indices.push_back( 3 );
	indices.push_back( 4 );
	indices.push_back( 5 );
	indices.push_back( 6 );
	indices.push_back( 6 );
	indices.push_back( 7 );
	indices.push_back( 7 );
	indices.push_back( 5 );

	return {vb, indices, true};
}

TriangleMesh Geometry::makeCameraFrustum( const float width,
	const float height,
	const float nearZ,
	const float farZ )
{
	ver::VertexInputLayout vertexLayout;
	vertexLayout.add( ver::VertexInputLayout::Position3D );
	ver::VBuffer vb{std::move( vertexLayout )};
	{
		// A frustum requires 8 vertices.
		const float zRatio = farZ / nearZ;
		const float nearX = width / 2.0f;
		const float nearY = height / 2.0f;
		const float farX = nearX * zRatio;
		const float farY = nearY * zRatio;
		vb.emplaceVertex( dx::XMFLOAT3{-nearX, nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{nearX, nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{nearX, -nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-nearX, -nearY, nearZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-farX, farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{farX, farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{farX, -farY, farZ} );
		vb.emplaceVertex( dx::XMFLOAT3{-farX, -farY, farZ} );
	}

	std::vector<unsigned> indices;
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 1 );
	indices.push_back( 2 );
	indices.push_back( 2 );
	indices.push_back( 3 );
	indices.push_back( 3 );
	indices.push_back( 0 );
	indices.push_back( 4 );
	indices.push_back( 5 );
	indices.push_back( 5 );
	indices.push_back( 6 );
	indices.push_back( 6 );
	indices.push_back( 7 );
	indices.push_back( 7 );
	indices.push_back( 4 );
	indices.push_back( 0 );
	indices.push_back( 4 );
	indices.push_back( 1 );
	indices.push_back( 5 );
	indices.push_back( 2 );
	indices.push_back( 6 );
	indices.push_back( 3 );
	indices.push_back( 7 );

	return {vb, indices};
}