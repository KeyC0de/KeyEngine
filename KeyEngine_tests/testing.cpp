#include <array>
#include "testing.h"
#include "../KeyEngine/third_party/assimp/include/assimp/Importer.hpp"
#include "../KeyEngine/third_party/assimp/include/assimp/postprocess.h"
#include "../KeyEngine/inc/dynamic_constant_buffer.h"
#include "../KeyEngine/inc/window.h"
#include "../KeyEngine/inc/material_loader.h"
#include "../KeyEngine/inc/drawable.h"
#include "../KeyEngine/inc/d3d_utils.h"
#include "../KeyEngine/inc/index_buffer.h"
#include "../KeyEngine/inc/input_layout.h"
#include "../KeyEngine/inc/pixel_shader.h"
#include "../KeyEngine/inc/primitive_topology.h"
#include "../KeyEngine/inc/vertex_buffer.h"
#include "../KeyEngine/inc/vertex_shader.h"
#include "../KeyEngine/inc/render_target.h"
#include "../KeyEngine/inc/bitmap.h"
#include "../KeyEngine/inc/assertions_console.h"


namespace dx = DirectX;

void testDynamicMeshLoading()
{
	using namespace ver;

	Assimp::Importer imp;
	const auto pScene = imp.ReadFile( "assets/models/brick_wall/brick_wall.obj",
		aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_ConvertToLeftHanded
		| aiProcess_GenNormals
		| aiProcess_CalcTangentSpace );
	auto layout = VertexLayout{}
		.add( VertexLayout::Position3D )
		.add( VertexLayout::Normal )
		.add( VertexLayout::Tangent )
		.add( VertexLayout::Bitangent )
		.add( VertexLayout::Texture2D );
	ver::Buffer cb{std::move( layout ), *pScene->mMeshes[0]};

	for ( auto i = 0ull,
		end = cb.getVertexCount();
		i < end; ++i )
	{
		const auto a = cb[i].getMember<VertexLayout::Position3D>();
		const auto b = cb[i].getMember<VertexLayout::Normal>();
		const auto c = cb[i].getMember<VertexLayout::Tangent>();
		const auto d = cb[i].getMember<VertexLayout::Bitangent>();
		const auto e = cb[i].getMember<VertexLayout::Texture2D>();
	}
}


void testMaterialSystemLoading( Graphics& gph )
{
	std::string path = "assets/models/brick_wall/brick_wall.obj";
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile( path,
		aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_ConvertToLeftHanded
		| aiProcess_GenNormals
		| aiProcess_CalcTangentSpace );
	MaterialLoader mat{gph, *pScene->mMaterials[1], path};
	Drawable drawable{gph, mat, *pScene->mMeshes[0]};
}


void testScaleMatrixTranslation()
{
	auto tlMat = DirectX::XMMatrixTranslation( 20.f, 30.f, 40.f );
	tlMat = util::scaleTranslation( tlMat,
		0.1f );
	dx::XMFLOAT4X4 f4;
	dx::XMStoreFloat4x4( &f4, tlMat );
	auto etl = util::extractTranslation( f4 );
	ASSERT( etl.x == 2.f && etl.y == 3.f && etl.z == 4.f, "NO!" );
}


void testDynamicConstant()
{
	using namespace std::string_literals;
	// data roundtrip tests
	{
		con::RawLayout lay;
		lay.add<con::Struct>( "butts"s );
		lay["butts"s].add<con::Float3>( "pubes"s );
		lay["butts"s].add<con::Float>( "dank"s );
		lay.add<con::Float>( "woot"s );
		lay.add<con::Array>( "arr"s );
		lay["arr"s].set<con::Struct>( 4 );
		lay["arr"s].T().add<con::Float3>( "twerk"s );
		lay["arr"s].T().add<con::Array>( "werk"s );
		lay["arr"s].T()["werk"s].set<con::Float>( 6 );
		lay["arr"s].T().add<con::Array>( "meta"s );
		lay["arr"s].T()["meta"s].set<con::Array>( 6 );
		lay["arr"s].T()["meta"s].T().set<con::Matrix>( 4 );
		lay["arr"s].T().add<con::Bool>( "booler"s );

		// fails: duplicate symbol name
		// s.add<con::Bool>( "arr"s );

		// fails: bad symbol name
		//s.add<con::Bool>( "69man" );

		auto cb = con::Buffer( std::move( lay ) );

		// fails to compile: conversion not in topo map
		//cb["woot"s] = "#"s;

		const auto sig = cb.getRootLayoutElement().getSignature();

		{
			auto exp = 42.0f;
			cb["woot"s] = exp;
			float act = cb["woot"s];
			ASSERT( act == exp, "Assertion failed!" );
		}
		{
			auto exp = 420.0f;
			cb["butts"s]["dank"s] = exp;
			float act = cb["butts"s]["dank"s];
			ASSERT( act == exp, "Assertion failed!" );
		}
		{
			auto exp = 111.0f;
			cb["arr"s][2]["werk"s][5] = exp;
			float act = cb["arr"s][2]["werk"s][5];
			ASSERT( act == exp, "Assertion failed!" );
		}
		{
			auto exp = DirectX::XMFLOAT3{69.0f, 0.0f, 0.0f};
			cb["butts"s]["pubes"s] = exp;
			dx::XMFLOAT3 act = cb["butts"s]["pubes"s];
			ASSERT( !std::memcmp( &exp, &act, sizeof( DirectX::XMFLOAT3 ) ),
				"Assertion failed!" );
		}
		{
			DirectX::XMFLOAT4X4 exp;
			dx::XMStoreFloat4x4( &exp,
				dx::XMMatrixIdentity() );
			cb["arr"s][2]["meta"s][5][3] = exp;
			dx::XMFLOAT4X4 act = cb["arr"s][2]["meta"s][5][3];
			ASSERT( !std::memcmp( &exp, &act, sizeof( DirectX::XMFLOAT4X4 ) ),
				"Assertion failed!" );
		}
		{
			auto exp = true;
			cb["arr"s][2]["booler"s] = exp;
			bool act = cb["arr"s][2]["booler"s];
			ASSERT( act == exp, "Assertion failed!" );
		}
		{
			auto exp = false;
			cb["arr"s][2]["booler"s] = exp;
			bool act = cb["arr"s][2]["booler"s];
			ASSERT( act == exp, "Assertion failed!" );
		}
		// exists
		{
			ASSERT( cb["butts"s]["pubes"s].isValid(), "Assertion failed!" );
			ASSERT( !cb["butts"s]["fubar"s].isValid(), "Assertion failed!" );
			if ( auto ref = cb["butts"s]["pubes"s]; ref.isValid() )
			{
				dx::XMFLOAT3 f = ref;
				ASSERT( f.x == 69.0f, "Assertion failed!" );
			}
		}
		// set if exists
		{
			ASSERT( cb["butts"s]["pubes"s].setIfValid( dx::XMFLOAT3{1.0f, 2.0f, 3.0f} ),
				"Assertion failed!" );
			auto& f3 = static_cast<const dx::XMFLOAT3&>(cb["butts"s]["pubes"s]);
			ASSERT( f3.x == 1.0f && f3.y == 2.0f && f3.z == 3.0f, "Assertion failed!" );
			ASSERT( !cb["butts"s]["phubar"s].setIfValid( dx::XMFLOAT3{2.0f, 2.0f, 7.0f} ),
				"Assertion failed!" );
		}

		const auto& cb2 = cb;
		{
			dx::XMFLOAT4X4 act = cb2["arr"s][2]["meta"s][5][3];
			ASSERT( act._11 == 1.0f, "Assertion failed!" );
		}
		// this doesn't compile: buffer is const
		// cb["arr"][2]["booler"] = true;
		// static_cast<bool&>(cb["arr"][2]["booler"]) = true;

		// this fails ASSERTion: array out of bounds
		// cb["arr"s][200];

	}
	// size test array of arrays
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Array>( 6 );
		lay["arr"].T().set<con::Matrix>( 4 );
		auto cb = con::Buffer( std::move( lay ) );

		auto act = cb.getSizeInBytes();
		ASSERT( act == 16u * 4u * 4u * 6u, "Assertion failed!" );
	}
	// size test array of floats
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Float>( 16 );
		auto cb = con::Buffer( std::move( lay ) );

		auto act = cb.getSizeInBytes();
		ASSERT( act == 256u, "Assertion failed!" );
	}
	// size test array of structs with padding
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Struct>( 6 );
		lay["arr"s].T().add<con::Float2>( "a" );
		lay["arr"].T().add<con::Float3>( "b"s );
		auto cb = con::Buffer( std::move( lay ) );

		auto act = cb.getSizeInBytes();
		ASSERT( act == 16u * 2u * 6u, "Assertion failed!" );
	}
	// size test array of primitive that needs padding
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Float3>( 6 );
		auto cb = con::Buffer( std::move( lay ) );

		auto act = cb.getSizeInBytes();
		ASSERT( act == 16u * 6u, "Assertion failed!" );
	}
	// testing CookedLayout
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Float3>( 6 );
		auto cooked = con::LayoutMap::fetch( std::move( lay ) );
		// raw is cleared after donating
		lay.add<con::Float>( "arr" );
		// fails to compile, cooked returns const&
		// cooked["arr"].add<con::Float>("buttman");
		auto b1 = con::Buffer( cooked );
		b1["arr"][0] = dx::XMFLOAT3{ 69.0f, 0.0f, 0.0f };
		auto b2 = con::Buffer( cooked );
		b2["arr"][0] = dx::XMFLOAT3{ 420.0f, 0.0f, 0.0f };
		ASSERT( static_cast<dx::XMFLOAT3>(b1["arr"][0]).x == 69.0f, "Assertion failed!" );
		ASSERT( static_cast<dx::XMFLOAT3>(b2["arr"][0]).x == 420.0f, "Assertion failed!" );
	}
	// specific testing scenario (packing error)
	{
		con::RawLayout lay;
		lay.add<con::Float3>( "materialColor" );
		lay.add<con::Float3>( "modelSpecularColor" );
		lay.add<con::Float>( "modelSpecularGloss" );
		auto cooked = con::LayoutMap::fetch( std::move( lay ) );
		ASSERT( cooked.getSizeInBytes() == 48u, "Assertion failed!" );
	}
	// array non-packing
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Float>( 10 );
		auto cooked = con::LayoutMap::fetch( std::move( lay ) );
		ASSERT( cooked.getSizeInBytes() == 160u, "Assertion failed!" );
	}
	// array of struct w/ padding
	{
		con::RawLayout lay;
		lay.add<con::Array>( "arr" );
		lay["arr"].set<con::Struct>( 10 );
		lay["arr"].T().add<con::Float3>( "x" );
		lay["arr"].T().add<con::Float2>( "y" );
		auto cooked = con::LayoutMap::fetch( std::move( lay ) );
		ASSERT( cooked.getSizeInBytes() == 320u, "Assertion failed!" );
	}
	// testing pointer stuff
	{
		con::RawLayout lay;
		lay.add<con::Struct>( "butts"s );
		lay["butts"s].add<con::Float3>( "pubes"s );
		lay["butts"s].add<con::Float>( "dank"s );
		
		auto cb = con::Buffer( std::move( lay ) );
		const auto exp = 696969.6969f;
		cb["butts"s]["dank"s] = 696969.6969f;
		ASSERT( (float&)cb["butts"s]["dank"s] == exp, "Assertion failed!" );
		ASSERT( *(float*)&cb["butts"s]["dank"s] == exp, "Assertion failed!" );
		const auto exp2 = 42.424242f;
		*(float*)&cb["butts"s]["dank"s] = exp2;
		ASSERT( (float&)cb["butts"s]["dank"s] == exp2, "Assertion failed!" );
	}
	// specific testing scenario (packing error)
	{
		con::RawLayout lay;
		lay.add<con::Bool>( "normalMapEnabled" );
		lay.add<con::Bool>( "specularMapEnabled" );
		lay.add<con::Bool>( "hasGlossMap" );
		lay.add<con::Float>( "modelSpecularGloss" );
		lay.add<con::Float3>( "modelSpecularColor" );
		lay.add<con::Float>( "specularMapWeight" );

		auto cb = con::Buffer( std::move( lay ) );
		ASSERT( cb.getSizeInBytes() == 32u, "Assertion failed!" );
	}
	// specific testing scenario (array packing issues gimme a tissue)
	{
		const int maxRadius = 7;
		const int nCoef = maxRadius * 2 + 1;
		con::RawLayout lay;
		lay.add<con::Integer>( "nTaps" );
		lay.add<con::Array>( "coefficients" );
		lay["coefficients"].set<con::Float>( nCoef );
		con::Buffer cb{std::move( lay )};
		// ASSERT proper amount of memory allocated
		ASSERT( cb.getSizeInBytes() == ( nCoef + 1 ) * 4 * 4, "Assertion failed!" );
		// ASSERT array empty
		{
			const char* begin = reinterpret_cast<char*>( (int*)&cb["nTaps"] );
			ASSERT( std::all_of( begin,
						begin + cb.getSizeInBytes(),
						[]( char c )
						{
							return c == 0;
						} ),
				"Assertion failed!" );
		}
		// ASSERT sparse float storage
		{
			for ( int i = 0; i < nCoef; i++ )
			{
				cb["coefficients"][i] = 6.9f;
			}
			const auto begin = reinterpret_cast<std::array<float, 4>*>( (float*)&cb["coefficients"][0] );
			const auto end = begin + nCoef;
			ASSERT( std::all_of( begin,
						end,
						[]( const auto& arr )
						{
							return arr[0] == 6.9f
								&& arr[1] == 0.0f
								&& arr[2] == 0.0f
								&& arr[3] == 0.0f;
						} ),
				"Assertion failed!" );
		}
	}
}

void testDynamicVertex( Window& wnd )
{
	namespace dx = DirectX;
	using namespace ver;

	const auto renderWithVS = [&gph = wnd.getGraphics()]( const std::string& vsName )
	{
		const auto bitop = PrimitiveTopology::fetch( gph );

		// (dynamic) vertex buffer bindable assembly
		const auto layout = VertexLayout{}
			.add( VertexLayout::Position2D )
			.add( VertexLayout::Float3Color );

		ver::Buffer vb{layout, 3};
		vb[0].getMember<VertexLayout::Position2D>() = dx::XMFLOAT2{0.0f, 0.5f};
		vb[0].getMember<VertexLayout::Float3Color>() = dx::XMFLOAT3{1.0f, 0.0f, 0.0f};
		vb[1].getMember<VertexLayout::Position2D>() = dx::XMFLOAT2{0.5f, -0.5f};
		vb[1].getMember<VertexLayout::Float3Color>() = dx::XMFLOAT3{0.0f, 1.0f, 0.0f};
		vb[2].getMember<VertexLayout::Position2D>() = dx::XMFLOAT2{-0.5f, -0.5f};
		vb[2].getMember<VertexLayout::Float3Color>() = dx::XMFLOAT3{0.0f, 0.0f, 1.0f};
		const auto bivb = VertexBuffer::fetch( gph,
			"##?",
			vb );

		// other bindables..
		const std::vector<unsigned> idx = {0, 1, 2};
		const auto biidx = IndexBuffer::fetch( gph,
			"##?",
			idx );

		const auto bips = PixelShader::fetch( gph,
			"cube_ps.cso" );

		const auto bivs = VertexShader::fetch( gph,
			vsName );
		const auto bilay = InputLayout::fetch( gph,
			layout,
			*bivs );

		auto rt = RenderTargetShaderInput{gph, 1280, 720, 0};

		biidx->bind( gph );
		bivb->bind( gph );
		bitop->bind( gph );
		bips->bind( gph );
		bivs->bind( gph );
		bilay->bind( gph );
		rt.clear( gph, {0.0f, 0.0f, 0.0f, 1.0f} );
		rt.bindRenderSurface( gph );
		gph.drawIndexed( biidx->getIndexCount() );
		gph.shareRenderTarget()->bindRenderSurface( gph );
		rt.convertToBitmap( gph ).save( vsName + ".png" );
	};

	renderWithVS( "cube_vs.cso" );
}