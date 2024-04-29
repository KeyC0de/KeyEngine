#include "vertex_buffer.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


VertexBuffer::VertexBuffer( Graphics &gph,
	const ver::VBuffer &vb )
	:
	VertexBuffer(gph, "?", vb)
{

}

VertexBuffer::VertexBuffer( Graphics &gph,
	const std::string &tag,
	const ver::VBuffer &vb )
	:
	m_stride{static_cast<unsigned>( vb.getLayout().getSizeInBytes() )},
	m_tag{tag},
	m_vertexLayout(vb.getLayout())
{
	D3D11_BUFFER_DESC vbDesc{};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.CPUAccessFlags = 0u;
	vbDesc.MiscFlags = 0u;
	vbDesc.ByteWidth = static_cast<unsigned>( vb.getSizeInBytes() );
	vbDesc.StructureByteStride = m_stride;

	D3D11_SUBRESOURCE_DATA subRscData{};
	subRscData.pSysMem = vb.getRawBytes();
	HRESULT hres = getDevice( gph )->CreateBuffer( &vbDesc, &subRscData, &m_pVertexBuffer );
	ASSERT_HRES_IF_FAILED;
}

const ver::VertexInputLayout& VertexBuffer::getLayout() const noexcept
{
	return m_vertexLayout;
}

void VertexBuffer::bind( Graphics &gph ) cond_noex
{
	const unsigned offset = 0u;
	getDeviceContext( gph )->IASetVertexBuffers( 0u, 1u, m_pVertexBuffer.GetAddressOf(), &m_stride, &offset );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<VertexBuffer> VertexBuffer::fetch( Graphics &gph,
	const std::string &tag,
	const ver::VBuffer &vb )
{
	ASSERT( tag != "?", "No VertexBuffer tag available!" );
	return BindableMap::fetch<VertexBuffer>( gph, tag, vb );
}

std::string VertexBuffer::getUid() const noexcept
{
	return calcUid( m_tag );
}