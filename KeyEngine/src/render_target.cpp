#include "render_target.h"
#include "depth_stencil_view.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"
#include "bindable_exception.h"
#include "viewport.h"
#include "utils.h"


namespace mwrl = Microsoft::WRL;

IRenderTargetView::IRenderTargetView( Graphics &gph,
	const unsigned width,
	const unsigned height )
	:
	m_width(width),
	m_height(height)
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1u;
	texDesc.ArraySize = 1u;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1u;
	texDesc.SampleDesc.Quality = 0u;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0u;
	texDesc.MiscFlags = 0u;

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc,
		nullptr,
		&pTex );
	ASSERT_HRES_IF_FAILED;

	// create the rtv on the RTtexture
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{0};
	hres = getDevice( gph )->CreateRenderTargetView( pTex.Get(),
		&rtvDesc,
		&m_pRtv );
	ASSERT_HRES_IF_FAILED;
}

IRenderTargetView::IRenderTargetView( Graphics &gph,
	ID3D11Texture2D *pTex,
	std::optional<unsigned> face )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	pTex->GetDesc( &texDesc );

	m_width = texDesc.Width;
	m_height = texDesc.Height;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = texDesc.Format;
	if ( face.has_value() )
	{
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 1u;	// how many cube textures to create
		rtvDesc.Texture2DArray.FirstArraySlice = *face;
		rtvDesc.Texture2DArray.MipSlice = 0u;
	}
	else
	{
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D = D3D11_TEX2D_RTV{0};
	}

	HRESULT hres = getDevice( gph )->CreateRenderTargetView( pTex,
		&rtvDesc,
		&m_pRtv );
	ASSERT_HRES_IF_FAILED;
}

void IRenderTargetView::bindRenderSurface( Graphics &gph ) cond_noex
{
	bindRenderSurface( gph,
		static_cast<ID3D11DepthStencilView*>( nullptr ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	IRenderSurface *pRs ) cond_noex
{
	ASSERT( dynamic_cast<IDepthStencilView*>( pRs ) != nullptr, "Input Resource is not a IDepthStencilView!" );
	bindRenderSurface( gph,
		static_cast<IDepthStencilView*>( pRs ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	IDepthStencilView *pD3dDsv ) cond_noex
{
	bindRenderSurface( gph,
		pD3dDsv ?
			pD3dDsv->m_pDsv.Get() :
			nullptr );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	ID3D11DepthStencilView *pD3dDsv ) cond_noex
{
	auto viewport = Viewport::fetch( gph,
		(float) m_width,
		(float) m_height );
	viewport->bind( gph );

	getDeviceContext( gph )->OMSetRenderTargets( 1u,
		m_pRtv.GetAddressOf(),
		pD3dDsv );
	DXGI_GET_QUEUE_INFO( gph );
}

void IRenderTargetView::clear( Graphics &gph,
	const std::array<float, 4> &color ) cond_noex
{
	getDeviceContext( gph )->ClearRenderTargetView( m_pRtv.Get(),
		color.data() );
	DXGI_GET_QUEUE_INFO( gph );
}

void IRenderTargetView::release()
{
	m_pRtv->Release();
}

const unsigned IRenderTargetView::getWidth() const noexcept
{
	return m_width;
}

const unsigned IRenderTargetView::getHeight() const noexcept
{
	return m_height;
}

ID3D11RenderTargetView* IRenderTargetView::renderTargetView() const noexcept
{
	return m_pRtv.Get();
}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IRenderTargetView::createStagingTexture( Graphics &gph ) const
{
	mwrl::ComPtr<ID3D11Resource> pRtvRsc;
	m_pRtv->GetResource( &pRtvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pRtvTex;
	pRtvRsc.As( &pRtvTex );

	D3D11_TEXTURE2D_DESC rtvTexDesc{};
	pRtvTex->GetDesc( &rtvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = rtvTexDesc;
	stagingTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTexDesc.Usage = D3D11_USAGE_STAGING;
	stagingTexDesc.ArraySize = 1u;
	stagingTexDesc.BindFlags = 0u;
	stagingTexDesc.MiscFlags = 0u;

	mwrl::ComPtr<ID3D11Texture2D> pStagingTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &stagingTexDesc,
		nullptr,
		&pStagingTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	m_pRtv->GetDesc( &rtvDesc );
	// copy to staging texture
	if ( rtvDesc.ViewDimension == D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY )
	{
		// source pRtvTex is actually inside a cubemap texture,
		// use view info to find the correct slice and copy subresource
		getDeviceContext( gph )->CopySubresourceRegion( pStagingTex.Get(),
			0u,
			0u,
			0u,
			0u,
			pRtvTex.Get(),
			rtvDesc.Texture2DArray.FirstArraySlice,
			nullptr );
		DXGI_GET_QUEUE_INFO( gph );
	}
	else
	{
		getDeviceContext( gph )->CopyResource( pStagingTex.Get(),
			pRtvTex.Get() );
		DXGI_GET_QUEUE_INFO( gph );
	}

	return {std::move( pStagingTex ), stagingTexDesc};
}

const Bitmap IRenderTargetView::convertToBitmap( Graphics &gph,
	const unsigned width,
	const unsigned height ) const
{
	auto [pStagingTex, stagingTexDesc] = createStagingTexture( gph );

	if ( stagingTexDesc.Format != DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM )
	{
		THROW_BINDABLE_EXCEPTION( "DXGI_FORMAT must be DXGI_FORMAT_B8G8R8A8_UNORM !" );
	}

	// mapping texture and preparing vector
	Bitmap bitmap{width, height};

	D3D11_MAPPED_SUBRESOURCE msr{};
	HRESULT hres = getDeviceContext( gph )->Map( pStagingTex.Get(),
		0u,
		D3D11_MAP::D3D11_MAP_READ,
		0u,
		&msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gph );

	auto pStagingTexBytes = static_cast<const char*>( msr.pData );
	// iterate by Height then Width over the staging texture
	for ( unsigned int y = 0; y < height; ++y )
	{
		auto pRow = reinterpret_cast<const Bitmap::Texel*>( pStagingTexBytes + msr.RowPitch * y );
		for ( unsigned int x = 0; x < width; ++x )
		{
			bitmap.setTexel( x,
				y,
				*( pRow + x ) );
		}
	}
	getDeviceContext( gph )->Unmap( pStagingTex.Get(),
		0 );

	return bitmap;
}


RenderTargetShaderInput::RenderTargetShaderInput( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot )
	:
	IRenderTargetView{gph, width, height},
	m_slot(slot)
{
	mwrl::ComPtr<ID3D11Resource> pRsc;
	m_pRtv->GetResource( &pRsc );

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	HRESULT hres = getDevice( gph )->CreateShaderResourceView( pRsc.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;
}

void RenderTargetShaderInput::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}


RenderTargetOutput::RenderTargetOutput( Graphics &gph,
	ID3D11Texture2D *pTex,
	std::optional<unsigned> face )
	:
	IRenderTargetView{gph, pTex, face}
{

}

void RenderTargetOutput::bind( Graphics &gph ) cond_noex
{
	ASSERT( false, "Cannot bind RenderTargetOutput to the pipeline!" );
}