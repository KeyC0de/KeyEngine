#pragma once

#include "bindable.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "assertions_console.h"
#include "dxgi_info_queue.h"


//=============================================================
//	\class	IConstantBuffer
//	\author	KeyC0de
//	\date	2022/02/19 18:52
//	\brief	CB is the Constant Buffer struct we abstract away
//=============================================================
template<typename CB>
class IConstantBuffer
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pD3dCb;
	unsigned m_slot;
	static inline Graphics *s_gph;
public:
	//	\function	ctor	||	\date	2022/08/14 23:44
	//	\brief	initially empty d3dcb - supply the data on update
	IConstantBuffer( Graphics &gfx,
		const unsigned slot )
		:
		m_slot(slot)
	{
		s_gph = &gfx;

		D3D11_BUFFER_DESC cbDesc{};
		setBufferDesc( cbDesc );

		HRESULT hres = getDevice( gfx )->CreateBuffer( &cbDesc, nullptr, &m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}

	//	\function	IConstantBuffer	||	\date	2022/08/14 23:44
	//	\brief	ctor with the data supplied
	IConstantBuffer( Graphics &gfx,
		const CB &cb,
		const unsigned slot )
		:
		m_slot(slot)
	{
		s_gph = &gfx;

		//ASSERT( util::isAligned( &cb, 16 ), "Constant Buffer not 16B aligned!" );
		D3D11_BUFFER_DESC cbDesc{};
		setBufferDesc( cbDesc );

		D3D11_SUBRESOURCE_DATA SubRscData{};
		SubRscData.pSysMem = &cb;
		HRESULT hres = getDevice( gfx )->CreateBuffer( &cbDesc, &SubRscData, &m_pD3dCb );
		ASSERT_HRES_IF_FAILED;
	}

	//	\function	IConstantBuffer	||	\date	2022/08/14 19:45
	//	\brief	cheating constructor - the D3d resource has already been created (eg copied from another)
	IConstantBuffer( Microsoft::WRL::ComPtr<ID3D11Buffer> &d3dBuf,
		const unsigned slot )
		:
		m_pD3dCb{d3dBuf},
		m_slot(slot)
	{

	}

	//	\function	update	||	\date	2022/02/19 19:00
	//	\brief	Map, paste to msr, Unmap
	void update( Graphics &gfx,
		const CB &cb )
	{
		D3D11_MAPPED_SUBRESOURCE msr{};
		HRESULT hres = getDeviceContext( gfx )->Map( m_pD3dCb.Get(), 0u, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0u, &msr );
		ASSERT_HRES_IF_FAILED;
		DXGI_GET_QUEUE_INFO( gfx );

		memcpy( msr.pData, &cb, sizeof CB );

		getDeviceContext( gfx )->Unmap( m_pD3dCb.Get(), 0u );
	}

	ID3D11Buffer* getCb() const noexcept
	{
		return m_pD3dCb.Get();
	}

	unsigned getSlot() const noexcept
	{
		return m_slot;
	}

protected:
	static void setBufferDesc( D3D11_BUFFER_DESC &d3dBufDesc )
	{
		d3dBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		d3dBufDesc.Usage = D3D11_USAGE_DYNAMIC;
		d3dBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		d3dBufDesc.ByteWidth = sizeof CB;
		d3dBufDesc.MiscFlags = 0;
		d3dBufDesc.StructureByteStride = 0;
	}
};

template<typename CB>
class VertexShaderConstantBuffer final
	: public IConstantBuffer<CB>
{
	using IConstantBuffer<CB>::s_gph;
	using IConstantBuffer<CB>::m_pD3dCb;
	using IConstantBuffer<CB>::m_slot;

	using IBindable::getDevice;
	using IBindable::getDeviceContext;
	using IConstantBuffer<CB>::setBufferDesc;
public:
	using IConstantBuffer<CB>::IConstantBuffer;	// inheriting constructors
	using IConstantBuffer<CB>::getCb;
	using IConstantBuffer<CB>::getSlot;

	void bind( Graphics &gfx ) cond_noex override
	{
		getDeviceContext( gfx )->VSSetConstantBuffers( m_slot, 1u, m_pD3dCb.GetAddressOf() );
		DXGI_GET_QUEUE_INFO( gfx );
	}

	static std::shared_ptr<VertexShaderConstantBuffer> fetch( Graphics &gfx,
		const CB &cb,
		const unsigned slot )
	{
		return BindableMap::fetch<VertexShaderConstantBuffer>( gfx, cb, slot );
	}

	static std::shared_ptr<VertexShaderConstantBuffer> fetch( Graphics &gfx,
		const unsigned slot )
	{
		return BindableMap::fetch<VertexShaderConstantBuffer>( gfx, slot );
	}

	static std::string calcUid( const CB &cb,
		const unsigned slot )
	{
		return calcUid( slot );
	}

	static std::string calcUid( const unsigned slot )
	{
		using namespace std::string_literals;
		return typeid( VertexShaderConstantBuffer ).name() + "#"s + std::to_string( slot );
	}

	std::string getUid() const noexcept override
	{
		return calcUid( m_slot );
	}

	static VertexShaderConstantBuffer<CB> makeACopy( ID3D11Buffer *srcBuf,
		const unsigned slot )
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> destBuf;

		D3D11_BUFFER_DESC cbDesc{};
		setBufferDesc( cbDesc );

		HRESULT hres = getDevice( *s_gph )->CreateBuffer( &cbDesc, nullptr, &destBuf );
		ASSERT_HRES_IF_FAILED;

		getDeviceContext( *s_gph )->CopyResource( destBuf.Get(), srcBuf );
		DXGI_GET_QUEUE_INFO_P( s_gph );

		return VertexShaderConstantBuffer<CB>( destBuf, slot );
	}
};

template<typename CB>
class PixelShaderConstantBuffer final
	: public IConstantBuffer<CB>
{
	using IConstantBuffer<CB>::s_gph;
	using IConstantBuffer<CB>::m_pD3dCb;
	using IConstantBuffer<CB>::m_slot;

	using IBindable::getDevice;
	using IBindable::getDeviceContext;
	using IConstantBuffer<CB>::setBufferDesc;
public:
	using IConstantBuffer<CB>::IConstantBuffer;	// inheriting constructors
	using IConstantBuffer<CB>::getCb;
	using IConstantBuffer<CB>::getSlot;

	void bind( Graphics &gfx ) cond_noex override
	{
		getDeviceContext( gfx )->PSSetConstantBuffers( m_slot, 1u, m_pD3dCb.GetAddressOf() );
		DXGI_GET_QUEUE_INFO( gfx );
	}

	static std::shared_ptr<PixelShaderConstantBuffer> fetch( Graphics &gfx,
		const CB &cb,
		const unsigned slot )
	{
		return BindableMap::fetch<PixelShaderConstantBuffer>( gfx, cb, slot );
	}

	static std::shared_ptr<PixelShaderConstantBuffer> fetch( Graphics &gfx,
		const unsigned slot )
	{
		return BindableMap::fetch<PixelShaderConstantBuffer>( gfx, slot );
	}
	
	static std::string calcUid( const CB &cb,
		const unsigned slot )
	{
		return calcUid( slot );
	}

	static std::string calcUid( const unsigned slot )
	{
		using namespace std::string_literals;
		return typeid( PixelShaderConstantBuffer ).name() + "#"s + std::to_string( slot );
	}

	std::string getUid() const noexcept override
	{
		return calcUid( m_slot );
	}

	static PixelShaderConstantBuffer<CB> makeACopy( ID3D11Buffer *srcBuf,
		const unsigned slot )
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> destBuf;

		D3D11_BUFFER_DESC cbDesc{};
		setBufferDesc( cbDesc );

		HRESULT hres = getDevice( *s_gph )->CreateBuffer( &cbDesc, nullptr, &destBuf );
		ASSERT_HRES_IF_FAILED;

		getDeviceContext( *s_gph )->CopyResource( destBuf.Get(), srcBuf );
		DXGI_GET_QUEUE_INFO_P( s_gph );

		return PixelShaderConstantBuffer<CB>( destBuf, slot );
	}
};