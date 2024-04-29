#include "vertex_shader.h"
#include "bindable_map.h"
#include "utils.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


VertexShader::VertexShader( Graphics &gph,
	const std::string &filepath )
	:
	m_path{filepath}
{
#if defined _DEBUG && !defined NDEBUG
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/shaders/Debug/" + filepath ).c_str(), &m_pVsBlob );
#else
	HRESULT hres = D3DReadFileToBlob( util::s2ws( "int/x64/shaders/Release/" + filepath ).c_str(), &m_pVsBlob );
#endif
	ASSERT_HRES_IF_FAILED_MSG( filepath );

	hres = getDevice( gph )->CreateVertexShader( m_pVsBlob->GetBufferPointer(), m_pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader );
	ASSERT_HRES_IF_FAILED;
}

void VertexShader::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->VSSetShader( m_pVertexShader.Get(), nullptr, 0u );
	DXGI_GET_QUEUE_INFO( gph );
}

ID3DBlob* VertexShader::getBytecode() const noexcept
{
	return m_pVsBlob.Get();
}

std::shared_ptr<VertexShader> VertexShader::fetch( Graphics &gph,
	const std::string &filepath )
{
	return BindableMap::fetch<VertexShader>( gph, filepath );
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