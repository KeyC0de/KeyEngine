#include "render_target.h"
#include "graphics.h"
#include "texture_desc.h"
#include "depth_stencil_view.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"
#include "bindable_exception.h"
#include "viewport.h"
#include "utils.h"


namespace mwrl = Microsoft::WRL;

IRenderTargetView::IRenderTargetView( Graphics &gfx,
	ID3D11Texture2D *pTexture,
	std::optional<unsigned> face )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	pTexture->GetDesc( &texDesc );

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

	HRESULT hres = getDevice( gfx )->CreateRenderTargetView( pTexture, &rtvDesc, &m_pD3dRtv );
	ASSERT_HRES_IF_FAILED;
}

IRenderTargetView::IRenderTargetView( Graphics &gfx,
	const unsigned width,
	const unsigned height )
	:
	m_width(width),
	m_height(height)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	// create the view on the texture (buffer)
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{0};
	hres = getDevice( gfx )->CreateRenderTargetView( pTexture.Get(), &rtvDesc, &m_pD3dRtv );
	ASSERT_HRES_IF_FAILED;
}

void IRenderTargetView::unbind( Graphics &gfx ) noexcept
{
	getDeviceContext( gfx )->OMSetRenderTargets( 0, nullptr, nullptr );
	DXGI_GET_QUEUE_INFO( gfx );
}

void IRenderTargetView::bindRenderSurface( Graphics &gfx ) cond_noex
{
	bindRenderSurface( gfx, static_cast<ID3D11DepthStencilView*>( nullptr ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gfx,
	IRenderSurface *pRs ) cond_noex
{
	ASSERT( dynamic_cast<IDepthStencilView*>( pRs ) != nullptr, "Input Resource is not a IDepthStencilView!" );
	bindRenderSurface( gfx, static_cast<IDepthStencilView*>( pRs ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gfx,
	IDepthStencilView *pD3dDsv ) cond_noex
{
	bindRenderSurface( gfx, pD3dDsv ? pD3dDsv->m_pD3dDsv.Get() : nullptr );
}

void IRenderTargetView::bindRenderSurface( Graphics &gfx,
	ID3D11DepthStencilView *pD3dDsv ) cond_noex
{
	auto viewport = Viewport::fetch( gfx, (float) m_width, (float) m_height );
	viewport->bind( gfx );

	getDeviceContext( gfx )->OMSetRenderTargets( 1u, m_pD3dRtv.GetAddressOf(), pD3dDsv );
	DXGI_GET_QUEUE_INFO( gfx );
}

void IRenderTargetView::clear( Graphics &gfx,
	const std::array<float, 4> &color ) cond_noex
{
	getDeviceContext( gfx )->ClearRenderTargetView( m_pD3dRtv.Get(), color.data() );
	DXGI_GET_QUEUE_INFO( gfx );
}

void IRenderTargetView::clean( Graphics &gfx ) cond_noex
{
	// release the back buffer texture resource of the rtv
	mwrl::ComPtr<ID3D11Resource> pRtvRsc;
	m_pD3dRtv->GetResource( &pRtvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pRtvTex;
	pRtvRsc.As( &pRtvTex );

	pRtvTex.Reset();

	// release the rtv resource itself
	m_pD3dRtv.Reset();
}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IRenderTargetView::createStagingTexture( Graphics &gfx ) const
{
	mwrl::ComPtr<ID3D11Resource> pRtvRsc;
	m_pD3dRtv->GetResource( &pRtvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pRtvTex;
	pRtvRsc.As( &pRtvTex );

	D3D11_TEXTURE2D_DESC rtvTexDesc{};
	pRtvTex->GetDesc( &rtvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = createTextureDescriptor( rtvTexDesc.Width, rtvTexDesc.Height, rtvTexDesc.Format, (BindFlags) rtvTexDesc.BindFlags, CpuAccessFlags::CpuReadAccess, false, TextureUsage::Staging );

	mwrl::ComPtr<ID3D11Texture2D> pStagingTex;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &stagingTexDesc, nullptr, &pStagingTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	m_pD3dRtv->GetDesc( &rtvDesc );
	// copy to staging texture
	if ( rtvDesc.ViewDimension == D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY )
	{
		// source pRtvTex is actually inside a cubemap texture,
		// use view info to find the correct slice and copy subresource
		getDeviceContext( gfx )->CopySubresourceRegion( pStagingTex.Get(), 0u, 0u, 0u, 0u, pRtvTex.Get(), rtvDesc.Texture2DArray.FirstArraySlice, nullptr );
		DXGI_GET_QUEUE_INFO( gfx );
	}
	else
	{
		getDeviceContext( gfx )->CopyResource( pStagingTex.Get(), pRtvTex.Get() );
		DXGI_GET_QUEUE_INFO( gfx );
	}

	return {std::move( pStagingTex ), stagingTexDesc};
}

const Bitmap IRenderTargetView::convertToBitmap( Graphics &gfx,
	const unsigned width,
	const unsigned height ) const
{
	auto [pStagingTex, stagingTexDesc] = createStagingTexture( gfx );

	if ( stagingTexDesc.Format != DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM )
	{
		THROW_BINDABLE_EXCEPTION( "DXGI_FORMAT must be DXGI_FORMAT_B8G8R8A8_UNORM !" );
	}

	// mapping texture and preparing vector
	Bitmap bitmap{width, height};

	D3D11_MAPPED_SUBRESOURCE msr{};
	HRESULT hres = getDeviceContext( gfx )->Map( pStagingTex.Get(), 0u, D3D11_MAP::D3D11_MAP_READ, 0u, &msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gfx );

	auto pData = static_cast<const char*>( msr.pData );
	// iterate by Height then Width over the staging texture
	for ( unsigned int y = 0; y < height; ++y )
	{
		auto p = reinterpret_cast<const Bitmap::Texel*>( pData + msr.RowPitch * y );
		for ( unsigned int x = 0; x < width; ++x )
		{
			bitmap.setTexel( x, y, *( p + x ) );
		}
	}
	getDeviceContext( gfx )->Unmap( pStagingTex.Get(), 0 );

	return bitmap;
}

unsigned IRenderTargetView::getWidth() const noexcept
{
	return m_width;
}

unsigned IRenderTargetView::getHeight() const noexcept
{
	return m_height;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& IRenderTargetView::d3dResourceCom() noexcept
{
	return m_pD3dRtv;
}

ID3D11RenderTargetView* IRenderTargetView::d3dResource() const noexcept
{
	return m_pD3dRtv.Get();
}

void IRenderTargetView::setDebugObjectName( const char* name ) noexcept
{
#if defined _DEBUG && !defined NDEBUG
	m_pD3dRtv->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( name ), name );
#else
	(void)name;
#endif
}


RenderTargetShaderInput::RenderTargetShaderInput( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const RenderTargetViewMode rtvMode /*= DefaultRT*/ )
	:
	IRenderTargetView{gfx, width, height},
	m_slot(slot)
{
	mwrl::ComPtr<ID3D11Resource> pRsc;
	m_pD3dRtv->GetResource( &pRsc );

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getFormatRtv( rtvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	HRESULT hres = getDevice( gfx )->CreateShaderResourceView( pRsc.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

unsigned RenderTargetShaderInput::getSlot() const noexcept
{
	return m_slot;
}

void RenderTargetShaderInput::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}


RenderTargetOutput::RenderTargetOutput( Graphics &gfx,
	ID3D11Texture2D *pTexture,
	std::optional<unsigned> face /* = {} */ )
	:
	IRenderTargetView{gfx, pTexture, face}
{

}

void RenderTargetOutput::bind( Graphics &gfx ) cond_noex
{
	ASSERT( false, "Cannot bind RenderTargetOutput as shader input!" );
}