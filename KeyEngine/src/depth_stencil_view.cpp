#include "depth_stencil_view.h"
#include "texture_desc.h"
#include "render_target.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "viewport.h"
#include "bindable_exception.h"


namespace mwrl = Microsoft::WRL;

IDepthStencilView::IDepthStencilView( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const bool bBindAsShaderInput,
	const DepthStencilViewMode dsMode )
	:
	m_width(width),
	m_height(height)
{
	mwrl::ComPtr<ID3D11Texture2D> pTexture;

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width,
		height,
		getTypelessFormatDs( dsMode ),
		( bBindAsShaderInput ? BindFlags::DepthStencilTexture : BindFlags::DepthStencilTexture ),
		CpuAccessFlags::NoCpuAccess,
		false,
		TextureUsage::Default );

	HRESULT hres = getDevice( gph )->CreateTexture2D( &texDesc,
		nullptr,
		&pTexture );
	ASSERT_HRES_IF_FAILED;

	// create target view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc{};
	dsViewDesc.Format = getTypedFormatDs( dsMode );
	dsViewDesc.Flags = 0u;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u; // no mips with depth maps
	hres = getDevice( gph )->CreateDepthStencilView( pTexture.Get(),
		&dsViewDesc,
		&m_pD3dDsv);
	ASSERT_HRES_IF_FAILED;
}

IDepthStencilView::IDepthStencilView( Graphics &gph,
	ID3D11Texture2D *pTexture,
	const DepthStencilViewMode dsMode,
	std::optional<unsigned> face )
{
	D3D11_TEXTURE2D_DESC texDesc{};
	pTexture->GetDesc( &texDesc );
	m_width = texDesc.Width;
	m_height = texDesc.Height;

	// create ds view into the depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//dsvDesc.Flags = 0u;
	dsvDesc.Format = getTypedFormatDs( dsMode );
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

	HRESULT hres = getDevice( gph )->CreateDepthStencilView( pTexture,
		&dsvDesc,
		&m_pD3dDsv);
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
		m_pD3dDsv.Get() );
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
	const std::array<float, 4> &unused ) cond_noex
{
	getDeviceContext( gph )->ClearDepthStencilView( m_pD3dDsv.Get(),
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
	m_pD3dDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	pDsTex.Reset();

	// clean the dsv resource itself
	m_pD3dDsv.Reset();
}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>, D3D11_TEXTURE2D_DESC> IDepthStencilView::createStagingTexture( Graphics &gph ) const
{
	mwrl::ComPtr<ID3D11Resource> pDsvRsc;
	m_pD3dDsv->GetResource( &pDsvRsc );

	mwrl::ComPtr<ID3D11Texture2D> pDsTex;
	pDsvRsc.As( &pDsTex);

	D3D11_TEXTURE2D_DESC dsvTexDesc{};
	pDsTex->GetDesc( &dsvTexDesc );

	D3D11_TEXTURE2D_DESC stagingTexDesc = createTextureDescriptor( dsvTexDesc.Width,
		dsvTexDesc.Height,
		dsvTexDesc.Format,
		(BindFlags) dsvTexDesc.BindFlags,
		CpuAccessFlags::CpuReadAccess,
		false,
		TextureUsage::Staging );

	mwrl::ComPtr<ID3D11Texture2D> pStagingTex;
	HRESULT hres = getDevice( gph )->CreateTexture2D( &stagingTexDesc,
		nullptr,
		&pStagingTex );
	ASSERT_HRES_IF_FAILED;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	m_pD3dDsv->GetDesc( &dsvDesc );
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
	m_pD3dDsv->SetPrivateData( WKPDID_D3DDebugObjectName,
		(UINT) strlen( name ),
		name );
#endif
}


DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gph,
	const unsigned slot,
	const DepthStencilViewMode dsMode )
	:
	DepthStencilShaderInput(gph, gph.getClientWidth(), gph.getClientHeight(), slot, dsMode)
{

}

DepthStencilShaderInput::DepthStencilShaderInput( Graphics &gph,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsMode )
	:
	IDepthStencilView(gph, width, height, true, dsMode),
	m_slot(slot)
{
	mwrl::ComPtr<ID3D11Resource> pRes;
	m_pD3dDsv->GetResource( &pRes );

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDs( dsMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	HRESULT hres = getDevice( gph )->CreateShaderResourceView( pRes.Get(),
		&srvDesc,
		&m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;
}

unsigned DepthStencilShaderInput::getSlot() const noexcept
{
	return m_slot;
}

void DepthStencilShaderInput::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShaderResources( m_slot,
		1u,
		m_pD3dSrv.GetAddressOf() );
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
	IDepthStencilView(gph, width, height, false, DepthStencilViewMode::Normal)
{

}

DepthStencilOutput::DepthStencilOutput( Graphics &gph,
	ID3D11Texture2D *pTexture,
	const DepthStencilViewMode dsMode,
	std::optional<unsigned> face /* = {} */ )
	:
	IDepthStencilView{gph, pTexture, dsMode, face}
{

}

void DepthStencilOutput::bind( Graphics &gph ) cond_noex
{
	ASSERT( false, "DepthStencilOutput cannot be bound as a Shader Input." );
}