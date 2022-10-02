#include "depth_stencil_view.h"
#include "render_target.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "viewport.h"
#include "bindable_exception.h"


namespace mwrl = Microsoft::WRL;

// select the right dxgi format of the depth stencil view
// DXGI_FORMAT_R32_TYPELESS is compatible with both DXGI_FORMAT_D32_FLOAT and DXGI_FORMAT_R32_FLOAT
// _TYPELESS is actually a float type
// Use when creating the texture descriptor
const DXGI_FORMAT getTypelessFormat( const IDepthStencilView::Mode mode )
{
	switch ( mode )
	{
	case IDepthStencilView::Mode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT
	case IDepthStencilView::Mode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Typeless DXGI format in IDepthStencilView." );
}

// when we want to map a texture view on the Depth Stencil
const DXGI_FORMAT getTypedFormat( const IDepthStencilView::Mode mode )
{
	switch ( mode )
	{
	case IDepthStencilView::Mode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	case IDepthStencilView::Mode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Typed DXGI format in IDepthStencilView." );
}

// when we want to sample from the Depth Stencil in the back buffer
const DXGI_FORMAT getShaderInputFormat( const IDepthStencilView::Mode mode )
{
	switch ( mode )
	{
	case IDepthStencilView::Mode::Normal:
		return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case IDepthStencilView::Mode::ShadowDepth:
		return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
	}
	THROW_BINDABLE_EXCEPTION( "Invalid mode for Shader Input DXGI format in IDepthStencilView." );
}


IDepthStencilView::IDepthStencilView( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const bool bBindAsShaderInput,
	const Mode mode )
	:
	m_width(width),
	m_height(height)
{
	mwrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC dsTexDesc{};
	dsTexDesc.Width = width;
	dsTexDesc.Height = height;
	dsTexDesc.MipLevels = 1u;
	dsTexDesc.ArraySize = 1u;	// view on a single texture, not a texture cube (6 texs)
	dsTexDesc.Format = getTypelessFormat( mode );
	dsTexDesc.SampleDesc.Count = 1u;
	dsTexDesc.SampleDesc.Quality = 0u;
	dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
	dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | ( bBindAsShaderInput ?
		D3D11_BIND_SHADER_RESOURCE :
		0u );
	HRESULT hres = getDevice( gph )->CreateTexture2D( &dsTexDesc,
		nullptr,
		&pDepthStencil );
	ASSERT_HRES_IF_FAILED;

	// create target view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc{};
	dsViewDesc.Format = getTypedFormat( mode );
	dsViewDesc.Flags = 0u;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u; // no mips with depth maps
	hres = getDevice( gph )->CreateDepthStencilView( pDepthStencil.Get(),
		&dsViewDesc,
		&m_pDsv );
	ASSERT_HRES_IF_FAILED;
}

IDepthStencilView::IDepthStencilView( Graphics &gph,
	mwrl::ComPtr<ID3D11Texture2D> pTexture,
	const unsigned face )
{
	D3D11_TEXTURE2D_DESC descTex{};
	pTexture->GetDesc( &descTex );
	m_width = descTex.Width;
	m_height = descTex.Height;

	// create ds view into the depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = 0u;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0u;
	dsvDesc.Texture2DArray.ArraySize = 1u;
	dsvDesc.Texture2DArray.FirstArraySlice = face;
	HRESULT hres = getDevice( gph )->CreateDepthStencilView( pTexture.Get(),
		&dsvDesc,
		&m_pDsv );
	ASSERT_HRES_IF_FAILED;
}

void IDepthStencilView::bindRenderSurface( Graphics &gph ) cond_noex
{
	auto viewport = Viewport::fetch( gph,
		(float) m_width,
		(float) m_height );
	viewport->bind( gph );

	getDeviceContext( gph )->OMSetRenderTargets( 0u,
		nullptr,
		m_pDsv.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

void IDepthStencilView::bindRenderSurface( Graphics &gph,
	IRenderSurface *rt ) cond_noex
{
	ASSERT( dynamic_cast<IRenderTargetView*>( rt ) != nullptr, "Bad cast. IRenderSurface is not a IRenderTargetView!" );
	bindRenderSurface( gph,
		static_cast<IRenderTargetView*>( rt ) );
}

void IDepthStencilView::bindRenderSurface( Graphics &gph,
	IRenderTargetView *rt ) cond_noex
{
	rt->bindRenderSurface( gph,
		this );
}

void IDepthStencilView::clear( Graphics &gph,
	const std::array<float, 4>& unused ) cond_noex
{
	getDeviceContext( gph )->ClearDepthStencilView( m_pDsv.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0u );
	DXGI_GET_QUEUE_INFO( gph );
}

void IDepthStencilView::clean( Graphics &gph ) cond_noex
{
	// unbind the render target and any depth/stencil buffer
	getDeviceContext( gph )->OMSetRenderTargets( 0u,
		nullptr,
		nullptr );

	// clean the texture resource of the dsv
	mwrl::ComPtr<ID3D11Resource> pDsvRsc;
	m_pDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	pDsTex.Reset();

	// clean the dsv resource itself
	m_pDsv.Reset();
}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IDepthStencilView::createStagingTexture( Graphics &gph ) const
{
	mwrl::ComPtr<ID3D11Resource> pDsvRsc;
	m_pDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	D3D11_TEXTURE2D_DESC dsvTexDesc{};
	pDsTex->GetDesc( &dsvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = dsvTexDesc;
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

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	m_pDsv->GetDesc( &dsvDesc );
	// copy to staging texture
	if ( dsvDesc.ViewDimension == D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2DARRAY )
	{
		getDeviceContext( gph )->CopySubresourceRegion( pStagingTex.Get(),
			0u,
			0u,
			0u,
			0u,
			pDsTex.Get(),
			dsvDesc.Texture2DArray.FirstArraySlice,
			nullptr );
		DXGI_GET_QUEUE_INFO( gph );
	}
	else
	{
		getDeviceContext( gph )->CopyResource( pStagingTex.Get(),
			pDsTex.Get() );
		DXGI_GET_QUEUE_INFO( gph );
	}

	return {std::move( pStagingTex ), stagingTexDesc};
}

const Bitmap IDepthStencilView::convertToBitmap( Graphics &gph,
	const unsigned width,
	const unsigned height,
	bool bLinearize ) const
{
	auto [pStagingTex, stagingTexDesc] = createStagingTexture( gph );

	// create Bitmap and copy from staging texture to it
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
	for ( unsigned y = 0u; y < height; ++y )
	{
		struct Pixel
		{
			unsigned char data[4];
		};
		auto pRow = reinterpret_cast<const Pixel*>( pStagingTexBytes + msr.RowPitch * size_t( y ) );
		for ( unsigned int x = 0; x < width; ++x )
		{
			if ( stagingTexDesc.Format == DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS )
			{
				// mask out the last 8 bits - val becomes 24 bits - last Byte is 0
				const auto val = 0x00FFFFFF
					& *reinterpret_cast<const unsigned int*>( pRow + x );
				if ( bLinearize )
				{
					const auto normalized = (float)val / (float)0x00FFFFFF;
					const auto linearized = 0.01f / ( 1.01f - normalized );
					const auto col = unsigned char( linearized * 255.0f );
					bitmap.setTexel( x,
						y,
						{col, col, col} );
				}
				else
				{
					const unsigned char b = val & 0xFF;
					const unsigned char g = (val >> 8) & 0xFF;
					const unsigned char r = (val >> 16) & 0xFF;
					bitmap.setTexel( x,
						y,
						{r, g, b} );
				}
			}
			else if ( stagingTexDesc.Format == DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS )
			{
				const auto val = *reinterpret_cast<const float*>( pRow + x );
				if ( bLinearize )
				{
					const auto linearized = 0.01f / ( 1.01f - val );
					const auto col = unsigned char(linearized * 255.0f);
					bitmap.setTexel( x,
						y,
						{col, col, col} );
				}
				else
				{
					const auto col = unsigned char(val * 255.0f);
					bitmap.setTexel( x,
						y,
						{col, col, col} );
				}
			}
			else
			{
				getDeviceContext( gph )->Unmap( pStagingTex.Get(),
					0u );
				throw std::runtime_error{"Bad IDepthStencilView dxgi format when converting to Bitmap"};
			}
		}
	}
	getDeviceContext( gph )->Unmap( pStagingTex.Get(),
		0u );

	return bitmap;
}

const unsigned int IDepthStencilView::getWidth() const noexcept
{
	return m_width;
}

const unsigned int IDepthStencilView::getHeight() const noexcept
{
	return m_height;
}

DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gph,
	const unsigned slot,
	const Mode mode )
	:
	DepthStencilShaderInput(gph, gph.getClientWidth(), gph.getClientHeight(), slot, mode)
{

}

DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const Mode mode )
	:
	IDepthStencilView(gph, width, height, true, mode),
	m_slot(slot)
{
	mwrl::ComPtr<ID3D11Resource> pRes;
	m_pDsv->GetResource( &pRes );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormat( mode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	HRESULT hres = getDevice( gph )->CreateShaderResourceView( pRes.Get(),
		&srvDesc,
		&m_pSrv );
	ASSERT_HRES_IF_FAILED;
}

void DepthStencilShaderInput::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}


DepthStencilOutput::DepthStencilOutput( Graphics &gph )
	:
	DepthStencilOutput(gph, gph.getClientWidth(), gph.getClientHeight())
{

}

DepthStencilOutput::DepthStencilOutput( Graphics &gph,
	const unsigned width,
	const unsigned height )
	:
	IDepthStencilView(gph, width, height, false, Mode::Normal)
{

}

DepthStencilOutput::DepthStencilOutput( Graphics &gph,
	mwrl::ComPtr<ID3D11Texture2D> pTexture,
	const unsigned face )
	:
	IDepthStencilView(gph, std::move( pTexture ), face)
{

}

void DepthStencilOutput::bind( Graphics &gph ) cond_noex
{
	ASSERT( false, "DepthStencilOutput cannot be bound as a Shader Input." );
}