#include "index_buffer.h"
#include "graphics.h"
#include "bindable_registry.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


IndexBuffer::IndexBuffer( Graphics &gfx,
	const std::vector<unsigned> &indices )
	:
	IndexBuffer(gfx, "?", indices)
{

}

IndexBuffer::IndexBuffer( Graphics &gfx,
	const std::string &tag,
	const std::vector<unsigned> &indices )
	:
	m_tag(tag),
	m_count{static_cast<unsigned>( indices.size() )}
{
	D3D11_BUFFER_DESC bd{};
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = static_cast<unsigned>( m_count * sizeof( unsigned ) );	// pitch
	bd.StructureByteStride = sizeof( unsigned );							// stride

	D3D11_SUBRESOURCE_DATA subRscData{};
	subRscData.pSysMem = indices.data();
	HRESULT hres = getDevice( gfx )->CreateBuffer( &bd, &subRscData, &m_pIndexBuffer );
	ASSERT_HRES_IF_FAILED;
}

void IndexBuffer::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
	DXGI_GET_QUEUE_INFO( gfx );
}

unsigned IndexBuffer::getIndexCount() const noexcept
{
	return m_count;
}

std::shared_ptr<IndexBuffer> IndexBuffer::fetch( Graphics &gfx,
	const std::string &tag,
	const std::vector<unsigned> &indices )
{
	ASSERT( tag != "?", "Invalid tag!" );
	return BindableRegistry::fetch<IndexBuffer>( gfx, tag, indices );
}

std::string IndexBuffer::getUid() const noexcept
{
	return calcUid( m_tag );
}