#include "depth_stencil_view.h"
#include "texture_desc.h"
#include "render_target.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "viewport.h"
#include "bindable_exception.h"


namespace mwrl = Microsoft::WRL;

IDepthStencilView::IDepthStencilView( Graphics &gfx,
	ID3D11Texture2D *pTexture,
	const DepthStencilViewMode dsvMode,
	std::optional<unsigned> face )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	pTexture->GetDesc( &texDesc );
	m_width = texDesc.Width;
	m_height = texDesc.Height;

	// create ds view into the depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//dsvDesc.Flags = 0u;
	dsvDesc.Format = getTypedFormatDsv( dsvMode );
	if ( face.has_value() )
	{
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = 1u;	// how many cube textures to create
		dsvDesc.Texture2DArray.FirstArraySlice = *face;
		dsvDesc.Texture2DArray.MipSlice = 0u;
	}
	else
	{
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D = D3D11_TEX2D_DSV{0};
	}

	HRESULT hres = getDevice( gfx )->CreateDepthStencilView( pTexture, &dsvDesc, &m_pD3dDsv );
	ASSERT_HRES_IF_FAILED;
}

IDepthStencilView::IDepthStencilView( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const bool bBindAsShaderInput,
	const DepthStencilViewMode dsvMode )
	:
	m_width(width),
	m_height(height)
{
	mwrl::ComPtr<ID3D11Texture2D> pTexture;

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), ( bBindAsShaderInput ? BindFlags::DepthStencilTexture : BindFlags::DepthStencilTexture ), CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	// create target view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc{};
	dsViewDesc.Format = getTypedFormatDsv( dsvMode );
	dsViewDesc.Flags = 0u;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u; // no mips with depth maps
	hres = getDevice( gfx )->CreateDepthStencilView( pTexture.Get(), &dsViewDesc, &m_pD3dDsv );
	ASSERT_HRES_IF_FAILED;
}

void IDepthStencilView::bindRenderSurface( Graphics &gfx ) cond_noex
{
	auto viewport = Viewport::fetch( gfx, (float) m_width, (float) m_height );
	viewport->bind( gfx );

	getDeviceContext( gfx )->OMSetRenderTargets( 0u, nullptr, m_pD3dDsv.Get() );
	DXGI_GET_QUEUE_INFO( gfx );
}

void IDepthStencilView::bindRenderSurface( Graphics &gfx,
	IRenderSurface *rt ) cond_noex
{
	ASSERT( dynamic_cast<IRenderTargetView*>( rt ) != nullptr, "Bad cast. IRenderSurface is not a IRenderTargetView!" );
	bindRenderSurface( gfx, static_cast<IRenderTargetView*>( rt ) );
}

void IDepthStencilView::bindRenderSurface( Graphics &gfx,
	IRenderTargetView *rt ) cond_noex
{
	rt->bindRenderSurface( gfx, this );
}

void IDepthStencilView::clear( Graphics &gfx,
	const std::array<float, 4> &unused ) cond_noex
{
	getDeviceContext( gfx )->ClearDepthStencilView( m_pD3dDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0ui8 );
	DXGI_GET_QUEUE_INFO( gfx );
}

void IDepthStencilView::clean( Graphics &gfx ) cond_noex
{
	// unbind the render target and any depth/stencil buffer
	getDeviceContext( gfx )->OMSetRenderTargets( 0u, nullptr, nullptr );

	// clean the texture resource of the dsv
	mwrl::ComPtr<ID3D11Resource> pDsvRsc;
	m_pD3dDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	pDsTex.Reset();

	// clean the dsv resource itself
	m_pD3dDsv.Reset();
}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IDepthStencilView::createStagingTexture( Graphics &gfx ) const
{
	mwrl::ComPtr<ID3D11Resource> pDsvRsc;
	m_pD3dDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	D3D11_TEXTURE2D_DESC dsvTexDesc{};
	pDsTex->GetDesc( &dsvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = createTextureDescriptor( dsvTexDesc.Width, dsvTexDesc.Height, dsvTexDesc.Format, (BindFlags) dsvTexDesc.BindFlags, CpuAccessFlags::CpuReadAccess, false, TextureUsage::Staging );

	mwrl::ComPtr<ID3D11Texture2D> pStagingTex;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &stagingTexDesc, nullptr, &pStagingTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	m_pD3dDsv->GetDesc( &dsvDesc );
	// copy to staging texture
	if ( dsvDesc.ViewDimension == D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY )
	{
		getDeviceContext( gfx )->CopySubresourceRegion( pStagingTex.Get(), 0u, 0u, 0u, 0u, pDsTex.Get(), dsvDesc.Texture2DArray.FirstArraySlice, nullptr );
		DXGI_GET_QUEUE_INFO( gfx );
	}
	else
	{
		getDeviceContext( gfx )->CopyResource( pStagingTex.Get(), pDsTex.Get() );
		DXGI_GET_QUEUE_INFO( gfx );
	}

	return {std::move( pStagingTex ), stagingTexDesc};
}

const Bitmap IDepthStencilView::convertToBitmap( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	bool bLinearize /*= true*/ ) const
{
	auto [pStagingTex, stagingTexDesc] = createStagingTexture( gfx );

	// create Bitmap and copy from staging texture to it
	Bitmap bitmap{width, height};
	D3D11_MAPPED_SUBRESOURCE msr{};
	HRESULT hres = getDeviceContext( gfx )->Map( pStagingTex.Get(), 0u, D3D11_MAP::D3D11_MAP_READ, 0u, &msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gfx );

	// .pData points to row 0 and depth slice 0.
	// .RowPitch contains the value that the runtime adds to pData to move from row to row, where each row contains multiple pixels.
	// Note that the runtime might assign values to RowPitch and DepthPitch that are larger than anticipated because there might be padding between rows and depth
	auto pData = static_cast<const char*>( msr.pData );
	for ( unsigned y = 0u; y < height; ++y )
	{
		struct Pixel
		{
			unsigned char data[4];
		};
		auto p = reinterpret_cast<const Pixel*>( pData + msr.RowPitch * size_t( y ) );
		for ( unsigned int x = 0; x < width; ++x )
		{
			if ( stagingTexDesc.Format == DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS )
			{
				// mask out the last 8 bits - val becomes 24 bits - last Byte is 0
				const auto val = 0x00FFFFFF & *reinterpret_cast<const unsigned int*>( p + x );
				if ( bLinearize )
				{
					const auto normalized = (float)val / (float)0x00FFFFFF;
					const auto linearized = 0.01f / ( 1.01f - normalized );
					const auto col = unsigned char( linearized * 255.0f );
					bitmap.setTexel( x, y, {col, col, col} );
				}
				else
				{
					const unsigned char b = val & 0xFF;
					const unsigned char g = (val >> 8) & 0xFF;
					const unsigned char r = (val >> 16) & 0xFF;
					bitmap.setTexel( x, y, {r, g, b} );
				}
			}
			else if ( stagingTexDesc.Format == DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS )
			{
				const auto val = *reinterpret_cast<const float*>( p + x );
				if ( bLinearize )
				{
					const auto linearized = 0.01f / ( 1.01f - val );
					const auto col = unsigned char(linearized * 255.0f);
					bitmap.setTexel( x, y, {col, col, col} );
				}
				else
				{
					const auto col = unsigned char(val * 255.0f);
					bitmap.setTexel( x, y, {col, col, col} );
				}
			}
			else
			{
				getDeviceContext( gfx )->Unmap( pStagingTex.Get(), 0u );
				throw std::runtime_error{"Bad IDepthStencilView dxgi format when converting to Bitmap"};
			}
		}
	}
	getDeviceContext( gfx )->Unmap( pStagingTex.Get(), 0u );

	return bitmap;
}

unsigned int IDepthStencilView::getWidth() const noexcept
{
	return m_width;
}

unsigned int IDepthStencilView::getHeight() const noexcept
{
	return m_height;
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& IDepthStencilView::d3dResourceCom() noexcept
{
	return m_pD3dDsv;
}

ID3D11DepthStencilView* IDepthStencilView::d3dResource() const noexcept
{
	return m_pD3dDsv.Get();
}

void IDepthStencilView::setDebugObjectName( const char* name ) noexcept
{
#if defined _DEBUG && !defined NDEBUG
	m_pD3dDsv->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( name ), name );
#else
	(void)name;
#endif
}


DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gfx,
	const unsigned slot,
	const DepthStencilViewMode dsvMode )
	:
	DepthStencilShaderInput(gfx, gfx.getClientWidth(), gfx.getClientHeight(), slot, dsvMode)
{

}

DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode /*= DepthStencilViewMode::DefaultDS*/ )
	:
	IDepthStencilView(gfx, width, height, true, dsvMode),
	m_slot(slot)
{
	mwrl::ComPtr<ID3D11Resource> pRes;
	m_pD3dDsv->GetResource( &pRes );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	HRESULT hres = getDevice( gfx )->CreateShaderResourceView( pRes.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

unsigned DepthStencilShaderInput::getSlot() const noexcept
{
	return m_slot;
}

void DepthStencilShaderInput::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}


DepthStencilOutput::DepthStencilOutput( Graphics &gfx )
	:
	DepthStencilOutput(gfx, gfx.getClientWidth(), gfx.getClientHeight())
{

}

DepthStencilOutput::DepthStencilOutput( Graphics &gfx,
	const unsigned width,
	const unsigned height )
	:
	IDepthStencilView(gfx, width, height, false, DepthStencilViewMode::DefaultDS)
{

}

DepthStencilOutput::DepthStencilOutput( Graphics &gfx,
	ID3D11Texture2D *pTexture,
	const DepthStencilViewMode dsvMode,
	std::optional<unsigned> face /*= {}*/ )
	:
	IDepthStencilView{gfx, pTexture, dsvMode, face}
{

}

void DepthStencilOutput::bind( Graphics &gfx ) cond_noex
{
	ASSERT( false, "Cannot bind DepthStencilOutput as shader input!" );
}