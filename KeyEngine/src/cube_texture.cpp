#include "cube_texture.h"
#include "graphics.h"
#include "texture_desc.h"
#include "depth_stencil_view.h"
#include "render_target.h"
#include "dxgi_info_queue.h"
#include "os_utils.h"
#include "bindable_registry.h"
#include <array>
#include "assertions_console.h"


namespace mwrl = Microsoft::WRL;

CubeTexture::CubeTexture( Graphics &gfx,
	const std::string &path,
	const unsigned slot )
	:
	m_path{path},
	m_slot(slot)
{
	// load 6 bitmaps for the cube faces
	std::vector<Bitmap> bitmaps;
	bitmaps.reserve( nCubeFaces );
	std::array<std::string, nCubeFaces> orientations = {
		"posx",
		"negx",
		"posy",
		"negy",
		"posz",
		"negz",
	};
	for ( unsigned i = 0; i < nCubeFaces; ++i )
	{
		bitmaps.emplace_back( Bitmap::loadFromFile( path + orientations[i] + ".png" ) );
	}

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( bitmaps[0].getWidth(), bitmaps[0].getHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::TextureOnly, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, true );

	D3D11_SUBRESOURCE_DATA subRscData[nCubeFaces]{};
	for ( int i = 0; i < nCubeFaces; i++ )
	{
		subRscData[i].pSysMem = bitmaps[i].getData();
		subRscData[i].SysMemPitch = bitmaps[i].getPitch();
		subRscData[i].SysMemSlicePitch = 0u;
	}

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, subRscData, &pTexture );
	ASSERT_HRES_IF_FAILED;

	// create srv on the cubemap texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

void CubeTexture::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

const std::string& CubeTexture::getPath() const noexcept
{
	return m_path;
}

std::shared_ptr<CubeTexture> CubeTexture::fetch( Graphics &gfx,
	const std::string &filepath,
	const unsigned slot )
{
	return BindableRegistry::fetch<CubeTexture>( gfx, filepath, slot );
}

std::string CubeTexture::calcUid( const std::string &filepath,
	const unsigned slot )
{
	using namespace std::string_literals;
	return typeid( CubeTexture ).name() + "#"s + filepath + "#"s + std::to_string( slot );
}

std::string CubeTexture::getUid() const noexcept
{
	return calcUid( m_path, m_slot );
}

unsigned CubeTexture::getSlot() const noexcept
{
	return m_slot;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureOffscreenRT::CubeTextureOffscreenRT( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DXGI_FORMAT format )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, format, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, true );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	// create RTVs on the texture cube's faces for capturing
	for ( unsigned face = 0u; face < nCubeFaces; ++face )
	{
		m_renderTargetViews.push_back( std::make_shared<RenderTargetOutput>( gfx, pTexture.Get(), face ) );
	}
}

void CubeTextureOffscreenRT::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<RenderTargetOutput> CubeTextureOffscreenRT::shareRenderTarget( const size_t index ) const
{
	return m_renderTargetViews[index];
}

RenderTargetOutput* CubeTextureOffscreenRT::accessRenderTarget( const size_t index )
{
	return m_renderTargetViews[index].get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureOffscreenDS::CubeTextureOffscreenDS( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, true );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSVs on the texture cube's faces for capturing depth (for shadow mapping)
	for ( unsigned face = 0u; face < nCubeFaces; ++face )
	{
		m_depthStencilViews.push_back( std::make_shared<DepthStencilOutput>( gfx, pTexture.Get(), dsvMode, face ) );
	}
}

void CubeTextureOffscreenDS::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<DepthStencilOutput> CubeTextureOffscreenDS::shareDepthBuffer( const size_t faceIndex ) const
{
	return m_depthStencilViews[faceIndex];
}

DepthStencilOutput* CubeTextureOffscreenDS::accessDepthBuffer( const size_t faceIndex )
{
	return m_depthStencilViews[faceIndex].get();
}

unsigned CubeTextureOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureArrayOffscreenDS::CubeTextureArrayOffscreenDS( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode,
	const unsigned nCubeTextures )
	:
	m_slot(slot),
	m_nCubeTextures{nCubeTextures}
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, true, nCubeTextures );

	mwrl::ComPtr<ID3D11Texture2D> pTextureArray;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTextureArray );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.TextureCubeArray.NumCubes = nCubeTextures;
	srvDesc.TextureCubeArray.First2DArrayFace = 0u;				// start of the array for each cubemap
	srvDesc.TextureCubeArray.MostDetailedMip = 0u;
	srvDesc.TextureCubeArray.MipLevels = texDesc.MipLevels;
	ASSERT( srvDesc.TextureCubeArray.NumCubes == texDesc.ArraySize / 6 && srvDesc.TextureCubeArray.NumCubes == nCubeTextures, "Invalid amount of cube textures in array!" );
	hres = getDevice( gfx )->CreateShaderResourceView( pTextureArray.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	m_depthStencilViews.reserve( nCubeTextures );

	// create DSV texture array
	for ( unsigned i = 0u; i < nCubeTextures; ++i )
	{
		m_depthStencilViews.emplace_back();
		for ( unsigned face = 0u; face < nCubeFaces; ++face )
		{
			m_depthStencilViews[i][face] = std::make_shared<DepthStencilOutput>( gfx, pTextureArray.Get(), dsvMode, i, face );
		}
	}
}

void CubeTextureArrayOffscreenDS::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<DepthStencilOutput> CubeTextureArrayOffscreenDS::shareDepthBuffer( const size_t index,
	const size_t faceIndex ) const
{
	return m_depthStencilViews[index][faceIndex];
}

DepthStencilOutput* CubeTextureArrayOffscreenDS::accessDepthBuffer( const size_t index,
	const size_t faceIndex )
{
	return m_depthStencilViews[index][faceIndex].get();
}

unsigned CubeTextureArrayOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}

unsigned CubeTextureArrayOffscreenDS::getTextureCount() const noexcept
{
	return m_nCubeTextures;
}
