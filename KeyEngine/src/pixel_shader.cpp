#include "pixel_shader.h"
#include "graphics.h"
#include "bindable_map.h"
#include "utils.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


PixelShader::PixelShader( Graphics &gfx,
	const std::string &filepath )
	:
	m_path(filepath)
{
	using namespace std::string_literals;
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/"s + SOLUTION_CONFIGURATION_STR + "/shaders/"s + filepath ).c_str(), &m_pPsBlob );
	ASSERT_HRES_IF_FAILED;

	hres = getDevice( gfx )->CreatePixelShader( m_pPsBlob->GetBufferPointer(), m_pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader );
	ASSERT_HRES_IF_FAILED;
}

void PixelShader::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShader( m_pPixelShader.Get(), nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<PixelShader> PixelShader::fetch( Graphics &gfx,
	const std::string &filepath )
{
	return BindableMap::fetch<PixelShader>( gfx, filepath );
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
PixelShaderNull::PixelShaderNull( Graphics &gfx )
{

}

void PixelShaderNull::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetShader( nullptr, nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<PixelShaderNull> PixelShaderNull::fetch( Graphics &gfx )
{
	return BindableMap::fetch<PixelShaderNull>( gfx );
}

std::string PixelShaderNull::calcUid()
{
	using namespace std::string_literals;
	return typeid( PixelShaderNull ).name() + "#"s;
}

std::string PixelShaderNull::getUid() const noexcept
{
	return calcUid();
}