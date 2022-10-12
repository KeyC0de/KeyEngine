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
	const unsigned slot )
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

	paintTextureToBitmap( gph,
		m_pTex.Get(),
		bitmap );

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

	getDeviceContext( gph )->GenerateMips( m_pSrv.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

Texture::Texture( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot )
	:
	m_bDynamic{true},
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

#ifdef D2D_INTEROP
	// create the DXGI Surface for d2d interoperability
	hres = m_pTex->QueryInterface( __uuidof( IDXGISurface ),
		reinterpret_cast<void**>( gph.surface2d().GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;
#endif

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
}

void Texture::paintTextureToBitmap( Graphics &gph,
	ID3D11Texture2D *tex,
	const Bitmap &bitmap,
	const D3D11_BOX *destPortion /* = nullptr */)
{
	getDeviceContext( gph )->UpdateSubresource( tex,
		0u,
		destPortion,
		bitmap.getData(),
		bitmap.getPitch(),
		0u );
	DXGI_GET_QUEUE_INFO( gph );
}

void Texture::update( Graphics &gph ) cond_noex
{
	HRESULT hres;
	D3D11_MAPPED_SUBRESOURCE mappedCpuBufferTexture;
	hres = getDeviceContext( gph )->Map( m_pTex.Get(),
		0u,
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&mappedCpuBufferTexture );
	ASSERT_HRES_IF_FAILED;

	ColorBGRA *pDst = static_cast<ColorBGRA*>( mappedCpuBufferTexture.pData );
	const unsigned dstWidth = mappedCpuBufferTexture.RowPitch / sizeof ColorBGRA;
	const unsigned srcWidth = m_width;
	const unsigned nRowBytes = srcWidth * sizeof ColorBGRA;
	ColorBGRA *pSrc = gph.cpuBuffer();
	// copy 1 line at a time from cpuBuffer to dynamic GPU texture
	for ( unsigned y = 0u; y < m_height; ++y )
	{
		memcpy( &pDst[y * dstWidth],
			&pSrc[y * srcWidth],
			nRowBytes );
	}
	getDeviceContext( gph )->Unmap( m_pTex.Get(),
		0u );
}

void Texture::bind( Graphics &gph ) cond_noex
{
	if ( m_bDynamic )
	{
		update( gph );
	}
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

bool Texture::hasAlpha() const noexcept
{
	return m_bAlpha;
}

const std::string& Texture::getPath() const noexcept
{
	return m_path;
}

std::shared_ptr<Texture> Texture::fetch( Graphics &gph,
	const std::string &filepath,
	const unsigned slot )
{
	return BindableMap::fetch<Texture>( gph,
		filepath,
		slot );
}

std::string Texture::calcUid( const std::string &filepath,
	const unsigned slot )
{
	using namespace std::string_literals;
	return typeid( Texture ).name() + "#"s + filepath + "#"s + std::to_string( slot );
}

const std::string Texture::getUid() const noexcept
{
	return calcUid( m_path,
		m_slot );
}

#pragma warning( disable : 6011 )
void Texture::flipModelNormalMapsGreenChannel( const std::string &objPath )
{
	const auto rootPath = std::filesystem::path{objPath}.parent_path().string() + "/";

	// load scene from .obj file to get our list of normal maps in the materials
	Assimp::Importer importer;
	const auto pScene = importer.ReadFile( objPath.c_str(),
		0u );
	ASSERT( pScene, std::string{"aiScene is null!" + std::string{importer.GetErrorString()}}.c_str() );

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
#pragma warning( default : 6011 )

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
	const float thresholdMin,
	const float thresholdMax )
{
#if defined _DEBUG && !defined NDEBUG
	auto &console = KeyConsole::instance();
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
			auto &console = KeyConsole::instance();
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
			auto &console = KeyConsole::instance();
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
			bitmap.setTexel( x,
				y,
				col );
		}
	}
	bitmap.save( pathOut );
}

const unsigned Texture::calculateNumberOfMipMaps( const unsigned width,
	const unsigned height ) noexcept
{
	const float xSteps = std::ceil( log2( (float)width ) );
	const float ySteps = std::ceil( log2( (float)height ) );
	return (unsigned)std::max( xSteps,
		ySteps );
}




/*

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureRT::CubeTextureRT( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DXGI_FORMAT format )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1u;
	texDesc.ArraySize = 6u;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1u;
	texDesc.SampleDesc.Quality = 0u;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0u;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc,
		nullptr,
		&pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;

	// create RTVs on the texture cube's faces for capturing
	for ( unsigned face = 0u; face < 6u; ++face )
	{
		m_renderTargetViews.push_back( std::make_shared<RenderTargetOutput>( gph,
			pTex.Get(),
			face ) );
	}
}

void CubeTextureRT::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<RenderTargetOutput> CubeTextureRT::shareRenderTarget( const size_t index ) const
{
	return m_renderTargetViews[index];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureDS::CubeTextureDS( Graphics &gph,
	const unsigned size,
	const unsigned slot,
	const DXGI_FORMAT format )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.MipLevels = 1u;
	texDesc.ArraySize = 6u;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1u;
	texDesc.SampleDesc.Quality = 0u;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0u;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc,
		nullptr,
		&pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSVs on the texture cube's faces for capturing depth (for shadow mapping)
	for ( unsigned face = 0u; face < 6u; ++face )
	{
		m_depthStencilViews.push_back( std::make_shared<DepthStencilOutput>( gph,
			pTex,
			face ) );
	}
}

void CubeTextureDS::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<DepthStencilOutput> CubeTextureDS::shareDepthBuffer( const size_t index ) const
{
	return m_depthStencilViews[index];
}*/