#include "index_buffer.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


IndexBuffer::IndexBuffer( Graphics &gph,
	const std::vector<unsigned> &indices )
	:
	IndexBuffer(gph, "?", indices)
{

}

IndexBuffer::IndexBuffer( Graphics &gph,
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
	HRESULT hres = getDevice( gph )->CreateBuffer( &bd, &subRscData, &m_pIndexBuffer );
	ASSERT_HRES_IF_FAILED;
}

void IndexBuffer::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->IASetIndexBuffer( m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u );
	DXGI_GET_QUEUE_INFO( gph );
}

unsigned IndexBuffer::getIndexCount() const noexcept
{
	return m_count;
}

std::shared_ptr<IndexBuffer> IndexBuffer::fetch( Graphics &gph,
	const std::string &tag,
	const std::vector<unsigned> &indices )
{
	ASSERT( tag != "?", "Invalid tag!" );
	return BindableMap::fetch<IndexBuffer>( gph, tag, indices );
}

std::string IndexBuffer::getUid() const noexcept
{
	return calcUid( m_tag );
}