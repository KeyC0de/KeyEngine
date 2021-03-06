#include "input_layout.h"
#include "bindable_map.h"
#include "vertex_shader.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


InputLayout::InputLayout( Graphics &gph,
	const ver::VertexLayout &vertexLayout,
	const VertexShader &vs )
	:
	m_vertexLayout{vertexLayout}
{
	const auto layoutDescs = m_vertexLayout.getD3DInputElementDescs();
	const auto pBytecode = vs.getBytecode();

	HRESULT hres = getDevice( gph )->CreateInputLayout( layoutDescs.data(),
		(unsigned)layoutDescs.size(),
		pBytecode->GetBufferPointer(),
		pBytecode->GetBufferSize(),
		&m_pInputLayout );
	ASSERT_HRES_IF_FAILED;
}

const ver::VertexLayout InputLayout::getLayout() const noexcept
{
	return m_vertexLayout;
}

void InputLayout::bind( Graphics &gph ) cond_noex
{
	getContext( gph )->IASetInputLayout( m_pInputLayout.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<InputLayout> InputLayout::fetch( Graphics &gph,
		const ver::VertexLayout &vertexLayout,
	const VertexShader &vs )
{
	return BindableMap::fetch<InputLayout>( gph,
		vertexLayout,
		vs );
}

std::string InputLayout::generateUid( const ver::VertexLayout &vertexLayout,
	const VertexShader &vs )
{
	using namespace std::string_literals;
	return typeid( InputLayout ).name() + "#"s + vertexLayout.getSignature() + "#"s
		+ vs.getUid();
}

std::string InputLayout::getUid() const noexcept
{
	using namespace std::string_literals;
	return typeid( InputLayout ).name() + "#"s + m_vertexLayout.getSignature() + "#"s
		+ m_vertexShaderUID;
}