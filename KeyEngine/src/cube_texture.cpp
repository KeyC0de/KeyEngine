#include "cube_texture.h"
#include "depth_stencil_view.h"
#include "render_target.h"
#include "dxgi_info_queue.h"
#include "os_utils.h"


namespace mwrl = Microsoft::WRL;

CubeTexture::CubeTexture( Graphics &gph,
	const std::string &path,
	unsigned slot )
	:
	m_path{path},
	m_slot(slot)
{
	// load 6 bitmaps for the cube faces
	std::vector<Bitmap> bitmaps;
	bitmaps.reserve(6);
	for ( int i = 0; i < 6; ++i )
	{
		bitmaps.emplace_back( Bitmap::loadFromFile( path + std::to_string( i ) + ".png" ) );
	}

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = bitmaps[0].getWidth();
	texDesc.Height = bitmaps[0].getHeight();
	texDesc.MipLevels = 1u;
	texDesc.ArraySize = 6u;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1u;
	texDesc.SampleDesc.Quality = 0u;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0u;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SUBRESOURCE_DATA subRscData[6]{};
	for ( int i = 0; i < 6; i++ )
	{
		subRscData[i].pSysMem = bitmaps[i].dataConst();
		subRscData[i].SysMemPitch = bitmaps[i].getPitch();
		subRscData[i].SysMemSlicePitch = 0u;
	}

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc,
		subRscData,
		&pTex );
	ASSERT_HRES_IF_FAILED;

	// create srv on the cubemap texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gph )->CreateShaderResourceView( pTex.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;
}

void CubeTexture::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureRT::CubeTextureRT( Graphics &gph,
	unsigned width,
	unsigned height,
	unsigned slot,
	DXGI_FORMAT format )
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
	getContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<RenderTargetOutput> CubeTextureRT::shareRenderTarget( size_t index )
	const
{
	return m_renderTargetViews[index];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CubeTextureDS::CubeTextureDS( Graphics &gph,
	unsigned size,
	unsigned slot,
	DXGI_FORMAT format )
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

std::shared_ptr<DepthStencilOutput> CubeTextureDS::shareDepthBuffer( size_t index ) const
{
	return m_depthStencilViews[index];
}

void CubeTextureDS::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}