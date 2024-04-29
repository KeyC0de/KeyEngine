#include "texture.h"
#include "texture_desc.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "console.h"
#include "assertions_console.h"
#include <filesystem>
#include <sstream>


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

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( m_width, m_height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &m_pTex );
	ASSERT_HRES_IF_FAILED;

	paintTextureWithBitmap( gph, m_pTex.Get(), bitmap );

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
#pragma warning( disable: 4146 )
	srvDesc.Texture2D.MipLevels = -1u;
#pragma warning( default: 4146 )
	hres = getDevice( gph )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pSrv );
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
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::TextureOnly, CpuAccessFlags::CpuWriteAccess, false, TextureUsage::Dynamic );

	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &m_pTex );
	ASSERT_HRES_IF_FAILED;

#ifdef D2D_INTEROP
	// create the DXGI Surface for d2d interoperability
	hres = m_pTex->QueryInterface( __uuidof( IDXGISurface ), reinterpret_cast<void**>( gph.surface2d().GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;
#endif

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pSrv );
	ASSERT_HRES_IF_FAILED;
}

void Texture::paintTextureWithBitmap( Graphics &gph,
	ID3D11Texture2D *tex,
	const Bitmap &bitmap,
	const D3D11_BOX *destPortion /* = nullptr */ )
{
	getDeviceContext( gph )->UpdateSubresource( tex, 0u, destPortion, bitmap.getData(), bitmap.getPitch(), 0u );
	DXGI_GET_QUEUE_INFO( gph );
}

void Texture::update( Graphics &gph ) cond_noex
{
	HRESULT hres;
	D3D11_MAPPED_SUBRESOURCE msr{};
	hres = getDeviceContext( gph )->Map( m_pTex.Get(), 0u, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0u, &msr );
	ASSERT_HRES_IF_FAILED;

	ColorBGRA *pDst = static_cast<ColorBGRA*>( msr.pData );
	const unsigned dstWidth = msr.RowPitch / sizeof ColorBGRA;
	const unsigned srcWidth = m_width;
	const unsigned nRowBytes = srcWidth * sizeof ColorBGRA;
	ColorBGRA *pSrc = gph.cpuBuffer();
	// copy 1 line at a time from cpuBuffer to dynamic GPU texture
	for ( unsigned y = 0u; y < m_height; ++y )
	{
		memcpy( &pDst[y * dstWidth], &pSrc[y * srcWidth], nRowBytes );
	}
	getDeviceContext( gph )->Unmap( m_pTex.Get(), 0u );
}

void Texture::bind( Graphics &gph ) cond_noex
{
	if ( m_bDynamic )
	{
		update( gph );
	}
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
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
	return BindableMap::fetch<Texture>( gph, filepath, slot );
}

std::string Texture::calcUid( const std::string &filepath,
	const unsigned slot )
{
	using namespace std::string_literals;
	return typeid( Texture ).name() + "#"s + filepath + "#"s + std::to_string( slot );
}

std::string Texture::getUid() const noexcept
{
	return calcUid( m_path, m_slot );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TextureOffscreenRT::TextureOffscreenRT( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const RenderTargetViewMode rtvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getFormatRtv( rtvMode ), BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(), &srvDesc, &m_pSrv );

	// create RTV that will render on the created offscreen texture and on a next pass we can use this RTV to read that texture
	m_pRtv = std::make_shared<RenderTargetOutput>( gph, pTex.Get() );
}

void TextureOffscreenRT::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<RenderTargetOutput> TextureOffscreenRT::shareRenderTarget() const
{
	return m_pRtv;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& TextureOffscreenRT::innerD3dResource() noexcept
{
	return m_pRtv->d3dResourceCom();
}

unsigned TextureOffscreenRT::getSlot() const noexcept
{
	return m_slot;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
TextureOffscreenDS::TextureOffscreenDS( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(), &srvDesc, &m_pSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSV that will render on the created offscreen texture and on a next pass we can use this SRV to read that texture
	m_pDsv = std::make_shared<DepthStencilOutput>( gph, pTex.Get(), dsvMode );
}

void TextureOffscreenDS::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<DepthStencilOutput> TextureOffscreenDS::shareDepthBuffer() const
{
	return m_pDsv;
}

unsigned TextureOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& TextureOffscreenDS::innerD3dResource() noexcept
{
	return m_pDsv->d3dResourceCom();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

const unsigned TextureProcessor::calculateNumberOfMipMaps( const unsigned width,
	const unsigned height ) noexcept
{
	const float xSteps = std::ceil( log2( (float)width ) );
	const float ySteps = std::ceil( log2( (float)height ) );
	return (unsigned)std::max( xSteps, ySteps );
}