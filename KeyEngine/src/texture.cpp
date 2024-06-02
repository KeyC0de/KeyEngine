#include "texture.h"
#include "texture_desc.h"
#include "texture_processor.h"
#include "graphics.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


namespace mwrl = Microsoft::WRL;
namespace dx = DirectX;

Texture::Texture( Graphics &gfx,
	const std::string &filepath,
	const unsigned slot,
	TextureOp op /*= nullptr*/  )
	:
	m_path{filepath},
	m_slot(slot),
	m_op(op)
{
	// #TODO: the rendering pipeline should not involve code paths that require loading assets from disk.
	// so preload bitmaps & shaders
	auto bitmap = Bitmap::loadFromFile( filepath );
	m_bAlpha = bitmap.hasAlpha();
	m_width = bitmap.getWidth();
	m_height = bitmap.getHeight();

	if ( op )
	{
		TextureProcessor::transformBitmap( bitmap, op );
	}

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( m_width, m_height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &m_pTex );
	ASSERT_HRES_IF_FAILED;

	paintTextureWithBitmap( gfx, m_pTex.Get(), bitmap );

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
#pragma warning( disable: 4146 )
	srvDesc.Texture2D.MipLevels = -1u;	// generate all Mips from #(MostDetailedMip) down to least detailed
#pragma warning( default: 4146 )
	hres = getDevice( gfx )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pSrv );
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	using namespace std::string_literals;
	const std::string srvName = "srv_filepath"s + "@"s + std::to_string( slot );
	const auto cstr = srvName.c_str();
	m_pSrv->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( cstr ), cstr );
#endif

	getDeviceContext( gfx )->GenerateMips( m_pSrv.Get() );
	DXGI_GET_QUEUE_INFO( gfx );
}

Texture::Texture( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	TextureOp op /*= nullptr*/  )
	:
	m_bDynamic{true},
	m_width(width),
	m_height(height),
	m_slot(slot),
	m_op(op)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::TextureOnly, CpuAccessFlags::CpuWriteAccess, false, TextureUsage::Dynamic );

	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &m_pTex );
	ASSERT_HRES_IF_FAILED;

#ifdef D2D_ONLY
	// create the DXGI Surface for d2d interoperability
	hres = m_pTex->QueryInterface( __uuidof( IDXGISurface ), reinterpret_cast<void**>( gfx.surface2d().GetAddressOf() ) );
	ASSERT_HRES_IF_FAILED;
#endif

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pSrv );
	ASSERT_HRES_IF_FAILED;
}

void Texture::paintTextureWithBitmap( Graphics &gfx,
	ID3D11Texture2D *tex,
	const Bitmap &bitmap,
	const D3D11_BOX *destPortion /* = nullptr */ )
{
	getDeviceContext( gfx )->UpdateSubresource( tex, 0u, destPortion, bitmap.getData(), bitmap.getPitch(), 0u );
	DXGI_GET_QUEUE_INFO( gfx );
}

void Texture::update( Graphics &gfx ) cond_noex
{
	HRESULT hres;
	D3D11_MAPPED_SUBRESOURCE msr{};
	hres = getDeviceContext( gfx )->Map( m_pTex.Get(), 0u, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0u, &msr );
	ASSERT_HRES_IF_FAILED;

	ColorBGRA *pDst = static_cast<ColorBGRA*>( msr.pData );
	const unsigned dstWidth = msr.RowPitch / sizeof ColorBGRA;
	const unsigned srcWidth = m_width;
	const unsigned nRowBytes = srcWidth * sizeof ColorBGRA;
	ColorBGRA *pSrc = gfx.cpuBuffer();
	// copy 1 line at a time from cpuBuffer to dynamic GPU texture
	for ( unsigned y = 0u; y < m_height; ++y )
	{
		memcpy( &pDst[y * dstWidth], &pSrc[y * srcWidth], nRowBytes );
	}
	getDeviceContext( gfx )->Unmap( m_pTex.Get(), 0u );
}

void Texture::bind( Graphics &gfx ) cond_noex
{
	if ( m_bDynamic )
	{
		update( gfx );
	}
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

bool Texture::hasAlpha() const noexcept
{
	return m_bAlpha;
}

const std::string& Texture::getPath() const noexcept
{
	return m_path;
}

unsigned Texture::getWidth() const noexcept
{
	return m_width;
}

unsigned Texture::getHeight() const noexcept
{
	return m_height;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& Texture::getD3dSrv()
{
	return m_pSrv;
}

std::shared_ptr<Texture> Texture::fetch( Graphics &gfx,
	const std::string &filepath,
	const unsigned slot,
	TextureOp op /*= nullptr*/  )
{
	return BindableMap::fetch<Texture>( gfx, filepath, slot, op );
}

std::string Texture::calcUid( const std::string &filepath,
	const unsigned slot,
	TextureOp op /*= nullptr*/ )
{
	using namespace std::string_literals;
	return typeid( Texture ).name() + "#"s + filepath + "#"s + std::to_string( slot ) + ( op != nullptr ? "#" + std::to_string( util::pointerToInt( std::addressof(op) ) ) : "" );
}

std::string Texture::getUid() const noexcept
{
	return calcUid( m_path, m_slot, m_op );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TextureOffscreenRT::TextureOffscreenRT( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const RenderTargetViewMode rtvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getFormatRtv( rtvMode ), BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pSrv );

	// create RTV that will render on the created offscreen texture and on a next pass we can use this RTV to read that texture
	m_pRtv = std::make_shared<RenderTargetOutput>( gfx, pTexture.Get() );
}

void TextureOffscreenRT::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<RenderTargetOutput> TextureOffscreenRT::shareRenderTarget() const
{
	return m_pRtv;
}

std::shared_ptr<RenderTargetOutput>& TextureOffscreenRT::rtv() noexcept
{
	return m_pRtv;
}

unsigned TextureOffscreenRT::getSlot() const noexcept
{
	return m_slot;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
TextureOffscreenDS::TextureOffscreenDS( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode )
	:
	m_slot(slot)
{
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, false, TextureUsage::Default );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSV that will render on the created offscreen texture and on a next pass we can use this SRV to read that texture
	m_pDsv = std::make_shared<DepthStencilOutput>( gfx, pTexture.Get(), dsvMode );
}

void TextureOffscreenDS::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<DepthStencilOutput> TextureOffscreenDS::shareDepthBuffer() const
{
	return m_pDsv;
}

std::shared_ptr<DepthStencilOutput>& TextureOffscreenDS::dsv() noexcept
{
	return m_pDsv;
}

unsigned TextureOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}
