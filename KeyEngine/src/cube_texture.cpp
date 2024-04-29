#include "cube_texture.h"
#include "texture_desc.h"
#include "depth_stencil_view.h"
#include "render_target.h"
#include "dxgi_info_queue.h"
#include "os_utils.h"


namespace mwrl = Microsoft::WRL;

CubeTexture::CubeTexture( Graphics &gph,
	const std::string &path,
	const unsigned slot )
	:
	m_path{path},
	m_slot(slot)
{
	// load 6 bitmaps for the cube faces
	std::vector<Bitmap> bitmaps;
	bitmaps.reserve( 6u );
	for ( int i = 0; i < 6; ++i )
	{
		bitmaps.emplace_back( Bitmap::loadFromFile( path + std::to_string( i ) + ".png" ) );
	}

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( bitmaps[0].getWidth(), bitmaps[0].getHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::TextureOnly, CpuAccessFlags::NoCpuAccess, true, TextureUsage::Default );

	D3D11_SUBRESOURCE_DATA subRscData[6]{};
	for ( int i = 0; i < 6; i++ )
	{
		subRscData[i].pSysMem = bitmaps[i].getData();
		subRscData[i].SysMemPitch = bitmaps[i].getPitch();
		subRscData[i].SysMemSlicePitch = 0u;
	}

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, subRscData, &pTex );
	ASSERT_HRES_IF_FAILED;

	// create srv on the cubemap texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

void CubeTexture::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureOffscreenRT::CubeTextureOffscreenRT( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DXGI_FORMAT format )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, format, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, true, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	// create RTVs on the texture cube's faces for capturing
	for ( unsigned face = 0u; face < 6u; ++face )
	{
		m_renderTargetViews.push_back( std::make_shared<RenderTargetOutput>( gph, pTex.Get(), face ) );
	}
}

void CubeTextureOffscreenRT::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<RenderTargetOutput> CubeTextureOffscreenRT::shareRenderTarget( const size_t index ) const
{
	return m_renderTargetViews[index];
}

RenderTargetOutput* CubeTextureOffscreenRT::renderTarget( const size_t index )
{
	return m_renderTargetViews[index].get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureOffscreenDS::CubeTextureOffscreenDS( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, true, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &pTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSVs on the texture cube's faces for capturing depth (for shadow mapping)
	for ( unsigned face = 0u; face < 6u; ++face )
	{
		m_depthStencilViews.push_back( std::make_shared<DepthStencilOutput>( gph, pTex.Get(), dsvMode, face ) );
	}
}

void CubeTextureOffscreenDS::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<DepthStencilOutput> CubeTextureOffscreenDS::shareDepthBuffer( const size_t index ) const
{
	return m_depthStencilViews[index];
}

DepthStencilOutput* CubeTextureOffscreenDS::depthBuffer( const size_t index )
{
	return m_depthStencilViews[index].get();
}