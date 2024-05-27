#include "texture_processor.h"
#include "texture.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "console.h"
#include "assertions_console.h"
#include <filesystem>
#include <sstream>


namespace dx = DirectX;

const unsigned TextureProcessor::calculateNumberOfMipMaps( const unsigned width,
	const unsigned height ) noexcept
{
	const float xSteps = std::ceil( log2( (float)width ) );
	const float ySteps = std::ceil( log2( (float)height ) );
	return (unsigned)std::max( xSteps, ySteps );
}

#pragma warning( disable : 6011 )
void TextureProcessor::flipModelNormalMapsGreenChannel( const std::string &objPath )
{
	const auto rootPath = std::filesystem::path{objPath}.parent_path().string() + "/";

	// load scene from .obj file to get our list of normal maps in the materials
	Assimp::Importer importer;
	const auto pScene = importer.ReadFile( objPath.c_str(), 0u );
	ASSERT( pScene, std::string{"aiScene is null!" + std::string{importer.GetErrorString()}}.c_str() );

	// loop through materials and process any normal maps
	for ( auto i = 0u; i < pScene->mNumMaterials; ++i )
	{
		const auto &mat = *pScene->mMaterials[i];
		aiString texFileName;
		if ( mat.GetTexture( aiTextureType_NORMALS, 0, &texFileName ) == aiReturn_SUCCESS )
		{
			const auto path = rootPath + texFileName.C_Str();
			flipNormalMapGreenChannel( path, path );
		}
	}
}
#pragma warning( default : 6011 )

void TextureProcessor::flipNormalMapGreenChannel( const std::string &pathIn,
	const std::string &pathOut )
{
	const auto normalOp = []( dx::XMVECTOR v, int x, int y ) -> dx::XMVECTOR
	{
		const auto flipY = dx::XMVectorSet( 1.0f, -1.0f, 1.0f, 1.0f );
		return dx::XMVectorMultiply( v, flipY );
	};
	transformFile( pathIn, pathOut, normalOp );
}

void TextureProcessor::validateNormalMap( const std::string &pathIn,
	const float thresholdMin,
	const float thresholdMax )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	console.log( "Validating normal map [" + pathIn + "]\n" );
#endif
	auto sum = dx::XMVectorZero();
	// function for processing each normal Texel
	const auto normalOp = [thresholdMin, thresholdMax, &sum]
		( dx::XMVECTOR v, int x, int y )
		-> dx::XMVECTOR
	{
		const float len = dx::XMVectorGetX( dx::XMVector3Length( v ) );
		const float z = dx::XMVectorGetZ( v );
		if ( len < thresholdMin || len > thresholdMax )
		{
			dx::XMFLOAT3 vec{};
			dx::XMStoreFloat3( &vec, v );
			std::ostringstream oss;
			oss << "Bad normal length: " << len << " at: (" << x << ", " << y << ") normal: (" << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
#if defined _DEBUG && !defined NDEBUG
			auto &console = KeyConsole::getInstance();
			console.log( oss.str() );
#endif
		}
		if ( z < 0.0f )
		{
			dx::XMFLOAT3 vec{};
			dx::XMStoreFloat3( &vec, v );
			std::ostringstream oss;
			oss << "Bad normal Z direction at: (" << x << ", " << y << ") normal: (" << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
#if defined _DEBUG && !defined NDEBUG
			auto &console = KeyConsole::getInstance();
			console.log( oss.str() );
#endif
		}
		sum = dx::XMVectorAdd( sum, v );
		return v;
	};
	// execute the validation for each texel
	auto bitmap = Bitmap::loadFromFile( pathIn );
	transformBitmap( bitmap, normalOp );
	// output bias
	{
		dx::XMFLOAT2 sumv{};
		dx::XMStoreFloat2( &sumv, sum );
		std::ostringstream oss;
		oss << "Normal map biases: (" << sumv.x << ", " << sumv.y << ")\n";
#if defined _DEBUG && !defined NDEBUG
		console.log( oss.str() );
#endif
	}
}

void TextureProcessor::makeStripes( const std::string &pathOut,
	const int size,
	const int stripeWidth )
{
	auto power = log2( size );
	ASSERT( modf( power, &power ) == 0.0, "Texture dimension is not a power of 2!" );
	ASSERT( stripeWidth < size / 2, "Not enough stripes to make!" );

	Bitmap bitmap( size, size );
	for ( int y = 0; y < size; ++y )
	{
		for ( int x = 0; x < size; ++x )
		{
			Bitmap::Texel col{0, 0, 0};
			if ( ( x / stripeWidth ) % 2 == 0 )
			{
				col = {255, 255, 255};
			}
			bitmap.setTexel( x, y, col );
		}
	}
	bitmap.save( pathOut );
}

void TextureProcessor::transformBitmap( Bitmap &bitmap,
	TextureOp f )
{
	const unsigned width = bitmap.getWidth();
	const unsigned height = bitmap.getHeight();
	for ( unsigned y = 0; y < height; ++y )
	{
		for ( unsigned x = 0; x < width; ++x )
		{
			const auto col = bitmap.getTexel( x, y );
			bitmap.setTexel( x, y, Bitmap::vectorToColor( f( col ) ) );
		}
	}
}
