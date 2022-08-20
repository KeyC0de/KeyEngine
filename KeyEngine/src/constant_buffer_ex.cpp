#include "constant_buffer_ex.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


IConstantBufferEx::IConstantBufferEx( Graphics &gph,
	unsigned slot,
	const con::CBElement &layoutRoot,
	const con::Buffer *pBuf )
	:
	m_slot(slot)
{
	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = (unsigned)layoutRoot.getSizeInBytes();
	if ( pBuf != nullptr )
	{
		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = pBuf->getRawBytes();
		HRESULT hres = getDevice( gph )->CreateBuffer( &cbDesc,
			&subData,
			&m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}
	else
	{
		HRESULT hres = getDevice( gph )->CreateBuffer( &cbDesc,
			nullptr,
			&m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}
}

void IConstantBufferEx::update( Graphics &gph,
	const con::Buffer &buf )
{
	ASSERT( &buf.getRootLayoutElement() == &getCBExRootLayoutElement(), "Input CB root element is not compatible!" );

	D3D11_MAPPED_SUBRESOURCE msr;
	HRESULT hres = getDeviceContext( gph )->Map( m_pD3dCb.Get(),
		0u,
		D3D11_MAP_WRITE_DISCARD,
		0u,
		&msr );
	ASSERT_HRES_IF_FAILED;
	DXGI_GET_QUEUE_INFO( gph );

	memcpy( msr.pData,
		buf.getRawBytes(),
		buf.getSizeInBytes() );

	getDeviceContext( gph )->Unmap( m_pD3dCb.Get(),
		0u );
}


void IVertexShaderConstantBufferEx::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->VSSetConstantBuffers( m_slot,
		1u,
		m_pD3dCb.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}


void IPixelShaderConstantBufferEx::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetConstantBuffers( m_slot,
		1u,
		m_pD3dCb.GetAddressOf() );
	DXGI_GET_QUEUE_INFO( gph );
}