#include "vertex_shader.h"
#include "bindable_map.h"
#include "utils.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


VertexShader::VertexShader( Graphics &gfx,
	const std::string &filepath )
	:
	m_path{filepath}
{
	using namespace std::string_literals;
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/"s + SOLUTION_CONFIGURATION_STR + "/shaders/"s + filepath ).c_str(), &m_pVsBlob );
	ASSERT_HRES_IF_FAILED_MSG( filepath );

	hres = getDevice( gfx )->CreateVertexShader( m_pVsBlob->GetBufferPointer(), m_pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader );
	ASSERT_HRES_IF_FAILED;
}

void VertexShader::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->VSSetShader( m_pVertexShader.Get(), nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gfx );
}

ID3DBlob* VertexShader::getBytecode() const noexcept
{
	return m_pVsBlob.Get();
}

std::shared_ptr<VertexShader> VertexShader::fetch( Graphics &gfx,
	const std::string &filepath )
{
	return BindableMap::fetch<VertexShader>( gfx, filepath );
}

std::string VertexShader::calcUid( const std::string &filepath )
{
	using namespace std::string_literals;
	return typeid( VertexShader ).name() + "#"s + filepath;
}

std::string VertexShader::getUid() const noexcept
{
	return calcUid( m_path );
}