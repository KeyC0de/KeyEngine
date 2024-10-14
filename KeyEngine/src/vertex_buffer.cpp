#include "vertex_buffer.h"
#include "graphics.h"
#include "bindable_registry.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "assertions_console.h"


VertexBuffer::VertexBuffer( Graphics &gfx,
	const ver::VBuffer &vb )
	:
	VertexBuffer(gfx, "?", vb)
{

}

VertexBuffer::VertexBuffer( Graphics &gfx,
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
	subRscData.pSysMem = vb.data();
	HRESULT hres = getDevice( gfx )->CreateBuffer( &vbDesc, &subRscData, &m_pVertexBuffer );
	ASSERT_HRES_IF_FAILED;
}

const ver::VertexInputLayout& VertexBuffer::getLayout() const noexcept
{
	return m_vertexLayout;
}

void VertexBuffer::bind( Graphics &gfx ) cond_noex
{
	const unsigned offset = 0u;
	getDeviceContext( gfx )->IASetVertexBuffers( 0u, 1u, m_pVertexBuffer.GetAddressOf(), &m_stride, &offset );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<VertexBuffer> VertexBuffer::fetch( Graphics &gfx,
	const std::string &tag,
	const ver::VBuffer &vb )
{
	ASSERT( tag != "?", "No VertexBuffer tag available!" );
	return BindableRegistry::fetch<VertexBuffer>( gfx, tag, vb );
}

std::string VertexBuffer::getUid() const noexcept
{
	return calcUid( m_tag );
}