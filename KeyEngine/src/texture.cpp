#include <filesystem>
#include <sstream>
#include "texture.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "console.h"
#include "assertions_console.h"


namespace mwrl = Microsoft::WRL;
namespace dx = DirectX;

Texture::Texture( Graphics &gph,
	const std::string &filepath,
	unsigned slot )
	:
	m_path{filepath},
	m_slot(slot)
{
	const auto bitmap = Bitmap::loadFromFile( filepath );
	m_bAlpha = bitmap.hasAlpha();
	m_width = bitmap.getWidth();
	m_height = bitmap.getHeight();

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT hres = getDevice( gph )->CreateTexture2D( &textureDesc,
		nullptr,
		&m_pTex );
	ASSERT_HRES_IF_FAILED;

	// write the image (bitmap) to the texture
	getContext( gph )->UpdateSubresource( m_pTex.Get(),
		0u,
		nullptr,
		bitmap.dataConst(),
		bitmap.getPitch(),
		0u );
	DXGI_GET_QUEUE_INFO( gph );

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
#pragma warning( disable: 4146 )
	srvDesc.Texture2D.MipLevels = -1u;
#pragma warning( default: 4146 )
	hres = getDevice( gph )->CreateShaderResourceView( m_pTex.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;

	getContext( gph )->GenerateMips( m_pSrv.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

Texture::Texture( Graphics &gph,
	unsigned width,
	unsigned height,
	unsigned slot )
	:
	m_width(width),
	m_height(height),
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC rtvTexDesc{};
	rtvTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	rtvTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	rtvTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	rtvTexDesc.Width = width;
	rtvTexDesc.Height = height;
	rtvTexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtvTexDesc.MipLevels = 1u;
	rtvTexDesc.ArraySize = 1u;
	rtvTexDesc.MiscFlags = 0u;
	rtvTexDesc.SampleDesc.Count = 1u;
	rtvTexDesc.SampleDesc.Quality = 0u;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &rtvTexDesc,
		nullptr,
		&m_pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_TEXTURE2D_DESC texDesc;
	m_pTex->GetDesc( &texDesc );
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( m_pTex.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;

	m_bDynamic = true;
}

void Texture::update( Graphics &gph ) cond_noex
{
	HRESULT hres;
	D3D11_MAPPED_SUBRESOURCE mappedCpuBufferTexture;
	hres = getContext( gph )->Map( m_pTex.Get(),
		0u,
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&mappedCpuBufferTexture );
	ASSERT_HRES_IF_FAILED;

	ColorBGRA *pDst = static_cast<ColorBGRA*>( mappedCpuBufferTexture.pData );
	const unsigned dstWidth = mappedCpuBufferTexture.RowPitch / sizeof ColorBGRA;
	const unsigned srcWidth = m_width;
	const unsigned nRowBytes = srcWidth * sizeof ColorBGRA;
	ColorBGRA *pSrc = gph.getCpuBuffer();
	// copy 1 line at a time from cpuBuffer to dynamic GPU texture
	for ( unsigned y = 0u; y < m_height; ++y )
	{
		memcpy( &pDst[y * dstWidth],
			&pSrc[y * srcWidth],
			nRowBytes );
	}
	getContext( gph )->Unmap( m_pTex.Get(),
		0u );
}

void Texture::bind( Graphics &gph ) cond_noex
{
	if ( m_bDynamic )
	{
		update( gph );
	}
	getContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

bool Texture::hasAlpha() const noexcept
{
	return m_bAlpha;
}

std::string Texture::getPath() const noexcept
{
	return m_path;
}

std::shared_ptr<Texture> Texture::fetch( Graphics &gph,
	const std::string &filepath,
	unsigned slot )
{
	return BindableMap::fetch<Texture>( gph,
		filepath,
		slot );
}

std::string Texture::generateUid( const std::string &filepath,
	unsigned slot )
{
	using namespace std::string_literals;
	return typeid( Texture ).name() + "#"s + filepath + "#"s + std::to_string( slot );
}

std::string Texture::getUid() const noexcept
{
	return generateUid( m_path,
		m_slot );
}


void Texture::flipModelNormalMapsGreenChannel( const std::string &objPath )
{
	const auto rootPath = std::filesystem::path{objPath}.parent_path().string() + "/";

	// load scene from .obj file to get our list of normal maps in the materials
	Assimp::Importer importer;
	const auto pScene = importer.ReadFile( objPath.c_str(),
		0u );
	ASSERT( pScene,
		std::string{"aiScene is null!" + std::string{importer.GetErrorString()}}.c_str() );

	// loop through materials and process any normal maps
	for ( auto i = 0u; i < pScene->mNumMaterials; ++i )
	{
		const auto &mat = *pScene->mMaterials[i];
		aiString texFileName;
		if ( mat.GetTexture( aiTextureType_NORMALS, 0, &texFileName ) == aiReturn_SUCCESS )
		{
			const auto path = rootPath + texFileName.C_Str();
			flipNormalMapGreenChannel( path,
				path );
		}
	}
}

void Texture::flipNormalMapGreenChannel( const std::string &pathIn,
	const std::string &pathOut )
{
	const auto normalOp = []( dx::XMVECTOR v, int x, int y ) -> dx::XMVECTOR
	{
		const auto flipY = dx::XMVectorSet( 1.0f, -1.0f, 1.0f, 1.0f );
		return dx::XMVectorMultiply( v,
			flipY );
	};
	transformFile( pathIn,
		pathOut,
		normalOp );
}

void Texture::validateNormalMap( const std::string &pathIn,
	float thresholdMin,
	float thresholdMax )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::getInstance();
	console.log( ( "Validating normal map [" + pathIn + "]\n" ).c_str() );
#endif
	auto sum = dx::XMVectorZero();
	// function for processing each normal Texel
	const auto normalOp = [thresholdMin, thresholdMax, &sum]
		( dx::XMVECTOR v, int x, int y ) -> dx::XMVECTOR
	{
		const float len = dx::XMVectorGetX( dx::XMVector3Length( v ) );
		const float z = dx::XMVectorGetZ( v );
		if ( len < thresholdMin || len > thresholdMax )
		{
			dx::XMFLOAT3 vec;
			dx::XMStoreFloat3( &vec,
				v );
			std::ostringstream oss;
			oss << "Bad normal length: "
				<< len
				<< " at: ("
				<< x
				<< ", "
				<< y
				<< ") normal: ("
				<< vec.x
				<< ", "
				<< vec.y
				<< ", "
				<< vec.z
				<< ")\n";
#if defined _DEBUG && !defined NDEBUG
			auto &console = KeyConsole::getInstance();
			console.log( oss.str().c_str() );
#endif
		}
		if ( z < 0.0f )
		{
			dx::XMFLOAT3 vec;
			dx::XMStoreFloat3( &vec, v );
			std::ostringstream oss;
			oss << "Bad normal Z direction at: ("
				<< x
				<< ", "
				<< y
				<< ") normal: ("
				<< vec.x
				<< ", "
				<< vec.y
				<< ", "
				<< vec.z
				<< ")\n";
#if defined _DEBUG && !defined NDEBUG
			auto &console = KeyConsole::getInstance();
			console.log( oss.str().c_str() );
#endif
		}
		sum = dx::XMVectorAdd( sum, v );
		return v;
	};
	// execute the validation for each texel
	auto bitmap = Bitmap::loadFromFile( pathIn );
	transformBitmap( bitmap,
		normalOp );
	// output bias
	{
		dx::XMFLOAT2 sumv;
		dx::XMStoreFloat2( &sumv,
			sum );
		std::ostringstream oss;
		oss << "Normal map biases: ("
			<< sumv.x
			<< ", "
			<< sumv.y
			<< ")\n";
#if defined _DEBUG && !defined NDEBUG
		console.log( oss.str().c_str() );
#endif
	}
}

void Texture::makeStripes( const std::string &pathOut,
	int size,
	int stripeWidth )
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
			bitmap.setTexel( x,
				y,
				col );
		}
	}
	bitmap.save( pathOut );
}

unsigned Texture::calculateNumberOfMipMaps( unsigned width,
	unsigned height ) noexcept
{
	const float xSteps = std::ceil( log2( (float)width ) );
	const float ySteps = std::ceil( log2( (float)height ) );
	return (unsigned)std::max( xSteps,
		ySteps );
}