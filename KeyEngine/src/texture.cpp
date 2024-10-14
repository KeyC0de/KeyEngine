#include "texture.h"
#include "texture_desc.h"
#include "texture_processor.h"
#include "graphics.h"
#include "bindable_registry.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


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

	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( m_width, m_height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, false );

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
	hres = getDevice( gfx )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

#if defined _DEBUG && !defined NDEBUG
	using namespace std::string_literals;
	const std::string srvName = "srv_filepath"s + "@"s + std::to_string( slot );
	const auto cstr = srvName.c_str();
	m_pD3dSrv->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( cstr ), cstr );
#endif

	getDeviceContext( gfx )->GenerateMips( m_pD3dSrv.Get() );
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
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, DXGI_FORMAT_B8G8R8A8_UNORM, BindFlags::TextureOnly, CpuAccessFlags::CpuWriteAccess, TextureUsage::Dynamic, false );

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
	hres = getDevice( gfx )->CreateShaderResourceView( m_pTex.Get(), &srvDesc, &m_pD3dSrv );
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
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
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
	return m_pD3dSrv;
}

std::shared_ptr<Texture> Texture::fetch( Graphics &gfx,
	const std::string &filepath,
	const unsigned slot,
	TextureOp op /*= nullptr*/  )
{
	return BindableRegistry::fetch<Texture>( gfx, filepath, slot, op );
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

unsigned Texture::getSlot() const noexcept
{
	return m_slot;
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
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getFormatRtv( rtvMode ), BindFlags::RenderTargetTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, false );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pD3dSrv );

	// create RTV that will render on the created offscreen texture and on a next pass we can use this RTV to read that texture
	m_pRtv = std::make_shared<RenderTargetOutput>( gfx, pTexture.Get() );
}

void TextureOffscreenRT::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<RenderTargetOutput> TextureOffscreenRT::shareRenderTarget() const
{
	return m_pRtv;
}

std::shared_ptr<RenderTargetOutput>& TextureOffscreenRT::accessRtv() noexcept
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
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, false );

	mwrl::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTexture );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = 1u;
	hres = getDevice( gfx )->CreateShaderResourceView( pTexture.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	// create DSV that will render on the created offscreen texture and on a next pass we can use this SRV to read that texture
	m_pDsv = std::make_shared<DepthStencilOutput>( gfx, pTexture.Get(), dsvMode, std::nullopt );
}

void TextureOffscreenDS::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<DepthStencilOutput> TextureOffscreenDS::shareDepthBuffer() const
{
	return m_pDsv;
}

std::shared_ptr<DepthStencilOutput>& TextureOffscreenDS::accessDsv() noexcept
{
	return m_pDsv;
}

unsigned TextureOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TextureArrayOffscreenDS::TextureArrayOffscreenDS( Graphics &gfx,
	const unsigned width,
	const unsigned height,
	const unsigned slot,
	const DepthStencilViewMode dsvMode,
	const unsigned nTextures )
	:
	m_slot(slot),
	m_nTextures{nTextures}
{
	// formats like DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS (typeless) & DXGI_FORMAT_R32_FLOAT (typed) support both depth-stencil operations and shader resource views
	D3D11_TEXTURE2D_DESC texDesc = createTextureDescriptor( width, height, getTypelessFormatDsv( dsvMode ), BindFlags::DepthStencilTexture, CpuAccessFlags::NoCpuAccess, TextureUsage::Default, false, nTextures );

	mwrl::ComPtr<ID3D11Texture2D> pTextureArray;
	HRESULT hres = getDevice( gfx )->CreateTexture2D( &texDesc, nullptr, &pTextureArray );
	ASSERT_HRES_IF_FAILED;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = getShaderInputFormatDsv( dsvMode );
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = nTextures;	// number of textures/slices in the array
	srvDesc.Texture2DArray.FirstArraySlice = 0u;	// start of the texture/slice in the array
	srvDesc.Texture2DArray.MostDetailedMip = 0u;
	srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
	ASSERT( srvDesc.Texture2DArray.ArraySize == texDesc.ArraySize && srvDesc.Texture2DArray.ArraySize == nTextures, "Invalid amount of textures in array!" );
	hres = getDevice( gfx )->CreateShaderResourceView( pTextureArray.Get(), &srvDesc, &m_pD3dSrv );
	ASSERT_HRES_IF_FAILED;

	m_depthStencilViews.reserve( nTextures );

	// create DSV texture array
	for ( unsigned i = 0u; i < nTextures; ++i )
	{
		m_depthStencilViews.push_back( std::make_shared<DepthStencilOutput>( gfx, pTextureArray.Get(), dsvMode, i, std::nullopt ) );
	}
}

void TextureArrayOffscreenDS::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShaderResources( m_slot, 1u, m_pD3dSrv.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<DepthStencilOutput> TextureArrayOffscreenDS::shareDepthBuffer( const size_t index ) const
{
	return m_depthStencilViews[index];
}

DepthStencilOutput* TextureArrayOffscreenDS::accessDepthBuffer( const size_t index )
{
	return m_depthStencilViews[index].get();
}

DepthStencilOutput* TextureArrayOffscreenDS::accessDsv( const size_t index ) noexcept
{
	return m_depthStencilViews[index].get();
}

unsigned TextureArrayOffscreenDS::getSlot() const noexcept
{
	return m_slot;
}

int TextureArrayOffscreenDS::getTextureCount() const noexcept
{
	return m_nTextures;
}
