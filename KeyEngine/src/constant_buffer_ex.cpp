#include "constant_buffer_ex.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


IConstantBufferEx::IConstantBufferEx( Graphics &gfx,
	const unsigned slot,
	const con::CBElement &layoutRoot,
	const con::CBuffer *pBuf )
	:
	m_slot(slot)
{
	D3D11_BUFFER_DESC d3dBufDesc{};
	setBufferDesc( d3dBufDesc, (unsigned) layoutRoot.getSizeInBytes() );
	if ( pBuf != nullptr )
	{
		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = pBuf->getRawBytes();
		HRESULT hres = getDevice( gfx )->CreateBuffer( &d3dBufDesc, &subData, &m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}
	else
	{
		HRESULT hres = getDevice( gfx )->CreateBuffer( &d3dBufDesc, nullptr, &m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}
}

void IConstantBufferEx::setBufferDesc( D3D11_BUFFER_DESC &d3dBufDesc,
	const unsigned byteWidth )
{
	d3dBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufDesc.ByteWidth = byteWidth;
	d3dBufDesc.MiscFlags = 0;
	d3dBufDesc.StructureByteStride = 0;
}

void IConstantBufferEx::update( Graphics &gfx,
	const con::CBuffer &cb )
{
	ASSERT( &cb.getRootElement() == &getCbRootElement(), "Input CB root element is not compatible!" );

	D3D11_MAPPED_SUBRESOURCE msr{};
	HRESULT hres = getDeviceContext( gfx )->Map( m_pD3dCb.Get(), 0u, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0u, &msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gfx );

	memcpy( msr.pData, cb.getRawBytes(), cb.getSizeInBytes() );

	getDeviceContext( gfx )->Unmap( m_pD3dCb.Get(), 0u );
}


void IVertexShaderConstantBufferEx::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->VSSetConstantBuffers( m_slot, 1u, m_pD3dCb.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}


void IPixelShaderConstantBufferEx::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->PSSetConstantBuffers( m_slot, 1u, m_pD3dCb.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gfx );
}