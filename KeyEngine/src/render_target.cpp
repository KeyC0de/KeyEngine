#include "render_target.h"
#include "texture_desc.h"
#include "depth_stencil_view.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"
#include "bindable_exception.h"
#include "viewport.h"
#include "utils.h"


namespace mwrl = Microsoft::WRL;

IRenderTargetView::IRenderTargetView( Graphics &gph,
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

	HRESULT hres = getDevice( gph )->CreateRenderTargetView( pTexture, &rtvDesc, &m_pD3dRtv );
	ASSERT_HRES_IF_FAILED;
}

IRenderTargetView::IRenderTargetView( Graphics &gph,
	const unsigned width,
	const unsigned height )
	:
	m_width(width),
	m_height(height)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc, nullptr, &pTex );
	ASSERT_HRES_IF_FAILED;

	// create the view on the texture (buffer)
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = D3D11_TEX2D_RTV{0};
	hres = getDevice( gph )->CreateRenderTargetView( pTex.Get(), &rtvDesc, &m_pD3dRtv );
	ASSERT_HRES_IF_FAILED;
}

void IRenderTargetView::unbind( Graphics &gph ) noexcept
{
	getDeviceContext( gph )->OMSetRenderTargets( 0, nullptr, nullptr );
	DXGI_GET_QUEUE_INFO( gph );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph ) cond_noex
{
	bindRenderSurface( gph, static_cast<ID3D11DepthStencilView*>( nullptr ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	IRenderSurface *pRs ) cond_noex
{
	ASSERT( dynamic_cast<IDepthStencilView*>( pRs ) != nullptr, "Input Resource is not a IDepthStencilView!" );
	bindRenderSurface( gph, static_cast<IDepthStencilView*>( pRs ) );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	IDepthStencilView *pD3dDsv ) cond_noex
{
	bindRenderSurface( gph, pD3dDsv ? pD3dDsv->m_pD3dDsv.Get() : nullptr );
}

void IRenderTargetView::bindRenderSurface( Graphics &gph,
	ID3D11DepthStencilView *pD3dDsv ) cond_noex
{
	Viewport viewport{gph, (float) m_width, (float) m_height};
	viewport.bind( gph );

	getDeviceContext( gph )->OMSetRenderTargets( 1u, m_pD3dRtv.GetAddressOf(), pD3dDsv );
	DXGI_GET_QUEUE_INFO( gph );
}

void IRenderTargetView::clear( Graphics &gph,
	const std::array<float, 4> &color ) cond_noex
{
	getDeviceContext( gph )->ClearRenderTargetView( m_pD3dRtv.Get(), color.data() );
	DXGI_GET_QUEUE_INFO( gph );
}

void IRenderTargetView::clean( Graphics &gph ) cond_noex
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

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IRenderTargetView::createStagingTexture( Graphics &gph ) const
{
	mwrl::ComPtr<ID3D11Resource> pRtvRsc;
	m_pD3dRtv->GetResource( &pRtvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pRtvTex;
	pRtvRsc.As( &pRtvTex );

	D3D11_TEXTURE2D_DESC rtvTexDesc{};
	pRtvTex->GetDesc( &rtvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = createTextureDescriptor( rtvTexDesc.Width, rtvTexDesc.Height, rtvTexDesc.Format, (BindFlags) rtvTexDesc.BindFlags, CpuAccessFlags::CpuReadAccess, false, TextureUsage::Staging );

	mwrl::ComPtr<ID3D11Texture2D> pStagingTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &stagingTexDesc, nullptr, &pStagingTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	m_pD3dRtv->GetDesc( &rtvDesc );
	// copy to staging texture
	if ( rtvDesc.ViewDimension == D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY )
	{
		// source pRtvTex is actually inside a cubemap texture,
		// use view info to find the correct slice and copy subresource
		getDeviceContext( gph )->CopySubresourceRegion( pStagingTex.Get(), 0u, 0u, 0u, 0u, pRtvTex.Get(), rtvDesc.Texture2DArray.FirstArraySlice, nullptr );
		DXGI_GET_QUEUE_INFO( gph );
	}
	else
	{
		getDeviceContext( gph )->CopyResource( pStagingTex.Get(), pRtvTex.Get() );
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
	HRESULT hres = getDeviceContext( gph )->Map( pStagingTex.Get(), 0u, D3D11_MAP::D3D11_MAP_READ, 0u, &msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gph );

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
	getDeviceContext( gph )->Unmap( pStagingTex.Get(), 0 );

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


RenderTargetShaderInput::RenderTargetShaderInput( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const RenderTargetViewMode rtvMode /*= DefaultRT*/ )
	:
	IRenderTargetView{gph, width, height},
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
	HRESULT hres = getDevice( gph )->CreateShaderResourceView( pRsc.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

unsigned RenderTargetShaderInput::getSlot() const noexcept
{
	return m_slot;
}

void RenderTargetShaderInput::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}


RenderTargetOutput::RenderTargetOutput( Graphics &gph,
	ID3D11Texture2D *pTex,
	std::optional<unsigned> face /* = {} */ )
	:
	IRenderTargetView{gph, pTex, face}
{

}

void RenderTargetOutput::bind( Graphics &gph ) cond_noex
{
	ASSERT( false, "Cannot bind RenderTargetOutput as shader input!" );
}