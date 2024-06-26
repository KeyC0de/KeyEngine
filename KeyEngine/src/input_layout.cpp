#include "input_layout.h"
#include "graphics.h"
#include "bindable_map.h"
#include "vertex_shader.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


InputLayout::InputLayout( Graphics &gfx,
	const ver::VertexInputLayout &vertexLayout,
	const VertexShader &vs )
	:
	m_vertexLayout{vertexLayout}
{
	const auto layoutDescs = m_vertexLayout.getD3DInputElementDescs();
	const auto pBytecode = vs.getBytecode();

	HRESULT hres = getDevice( gfx )->CreateInputLayout( layoutDescs.data(), (unsigned)layoutDescs.size(), pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), &m_pInputLayout );
	ASSERT_HRES_IF_FAILED;
}

const ver::VertexInputLayout InputLayout::getLayout() const noexcept
{
	return m_vertexLayout;
}

void InputLayout::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->IASetInputLayout( m_pInputLayout.Get() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<InputLayout> InputLayout::fetch( Graphics &gfx,
	const ver::VertexInputLayout &vertexLayout,
	const VertexShader &vs )
{
	return BindableMap::fetch<InputLayout>( gfx, vertexLayout, vs );
}

std::string InputLayout::calcUid( const ver::VertexInputLayout &vertexLayout,
	const VertexShader &vs )
{
	using namespace std::string_literals;
	return typeid( InputLayout ).name() + "#"s + vertexLayout.calcSignature() + "#"s + vs.getUid();
}

std::string InputLayout::getUid() const noexcept
{
	using namespace std::string_literals;
	return typeid( InputLayout ).name() + "#"s + m_vertexLayout.calcSignature() + "#"s + m_vertexShaderUID;
}