#include "pixel_shader.h"
#include "bindable_map.h"
#include "utils.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


PixelShader::PixelShader( Graphics &gph,
	const std::string &filepath )
	:
	m_path(filepath)
{
#if defined _DEBUG && !defined NDEBUG
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/shaders/Debug/" + filepath ).c_str(), &m_pPsBlob );
#else
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/shaders/Release/" + filepath ).c_str(), &m_pPsBlob );
#endif
	ASSERT_HRES_IF_FAILED;

	hres = getDevice( gph )->CreatePixelShader( m_pPsBlob->GetBufferPointer(), m_pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader );
	ASSERT_HRES_IF_FAILED;
}

void PixelShader::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShader( m_pPixelShader.Get(), nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<PixelShader> PixelShader::fetch( Graphics &gph,
	const std::string &filepath )
{
	return BindableMap::fetch<PixelShader>( gph, filepath );
}

ID3DBlob* PixelShader::getBytecode() const noexcept
{
	return m_pPsBlob.Get();
}

std::string PixelShader::calcUid( const std::string &filepath )
{
	using namespace std::string_literals;
	return typeid( PixelShader ).name() + "#"s + filepath;
}

std::string PixelShader::getUid() const noexcept
{
	return calcUid( m_path );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
PixelShaderNull::PixelShaderNull( Graphics &gph )
{

}

void PixelShaderNull::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShader( nullptr, nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<PixelShaderNull> PixelShaderNull::fetch( Graphics &gph )
{
	return BindableMap::fetch<PixelShaderNull>( gph );
}

std::string PixelShaderNull::calcUid()
{
	return typeid( PixelShaderNull ).name();
}

std::string PixelShaderNull::getUid() const noexcept
{
	return calcUid();
}