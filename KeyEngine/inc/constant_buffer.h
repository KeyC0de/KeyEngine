#pragma once

#include "bindable.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "assertions_console.h"
#include "dxgi_info_queue.h"


//=============================================================
//	\class	IConstantBuffer
//
//	\author	KeyC0de
//	\date	2022/02/19 18:52
//
//	\brief	CB is the Constant Buffer struct we abstract away
//=============================================================
template<typename CB>
class IConstantBuffer
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pCb;
	unsigned m_slot;
public:
	// empty cb
	IConstantBuffer( Graphics &gph,
		unsigned slot )
		:
		m_slot(slot)
	{
		D3D11_BUFFER_DESC cbDesc{};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.ByteWidth = sizeof CB;
		HRESULT hres = getDevice( gph )->CreateBuffer( &cbDesc,
			nullptr,
			&m_pCb );
		ASSERT_HRES_IF_FAILED;
	}

	IConstantBuffer( Graphics &gph,
		const CB &cb,
		unsigned slot )
		:
		m_slot(slot)
	{
		//ASSERT( util::isAligned( &cb, 16 ), "Constant Buffer not 16B aligned!" );
		D3D11_BUFFER_DESC cbDesc{};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.ByteWidth = sizeof cb;

		D3D11_SUBRESOURCE_DATA cbSubData{};
		cbSubData.pSysMem = &cb;
		HRESULT hres = getDevice( gph )->CreateBuffer( &cbDesc,
			&cbSubData,
			&m_pCb );
		ASSERT_HRES_IF_FAILED;
	}

	//===================================================
	//	\function	update
	//	\brief  Map, paste to msr, Unmap
	//	\date	2022/02/19 19:00
	void update( Graphics &gph,
		const CB &cb )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hres = getContext( gph )->Map( m_pCb.Get(),
			0u,
			D3D11_MAP_WRITE_DISCARD,
			0u,
			&msr );
		ASSERT_HRES_IF_FAILED;
		DXGI_GET_QUEUE_INFO( gph );

		memcpy( msr.pData,
			&cb,
			sizeof cb );

		getContext( gph )->Unmap( m_pCb.Get(),
			0u );
	}
};

template<typename CB>
class VertexShaderConstantBuffer final
	: public IConstantBuffer<CB>
{
	using IConstantBuffer<CB>::m_pCb;
	using IConstantBuffer<CB>::m_slot;
	using IBindable::getContext;
public:
	// inheriting constructors
	using IConstantBuffer<CB>::IConstantBuffer;

	void bind( Graphics &gph ) cond_noex override
	{
		getContext( gph )->VSSetConstantBuffers( m_slot,
			1u,
			m_pCb.GetAddressOf() );
		DXGI_GET_QUEUE_INFO( gph );
	}

	static std::shared_ptr<VertexShaderConstantBuffer> fetch( Graphics &gph,
		const CB &cb,
		unsigned slot )
	{
		return BindableMap::fetch<VertexShaderConstantBuffer>( gph,
			cb,
			slot );
	}

	static std::shared_ptr<VertexShaderConstantBuffer> fetch( Graphics &gph,
		unsigned slot )
	{
		return BindableMap::fetch<VertexShaderConstantBuffer>( gph,
			slot );
	}

	static std::string generateUid( const CB &cb,
		unsigned slot )
	{
		return generateUid( slot );
	}

	static std::string generateUid( unsigned slot )
	{
		using namespace std::string_literals;
		return typeid( VertexShaderConstantBuffer ).name() + "#"s + std::to_string( slot );
	}

	std::string getUid() const noexcept override
	{
		return generateUid( m_slot );
	}
};

template<typename CB>
class PixelShaderConstantBuffer final
	: public IConstantBuffer<CB>
{
	using IConstantBuffer<CB>::m_pCb;
	using IConstantBuffer<CB>::m_slot;
	using IBindable::getContext;
public:
	using IConstantBuffer<CB>::IConstantBuffer;

	void bind( Graphics &gph ) cond_noex override
	{
		getContext( gph )->PSSetConstantBuffers( m_slot,
			1u,
			m_pCb.GetAddressOf() );
		DXGI_GET_QUEUE_INFO( gph );
	}

	static std::shared_ptr<PixelShaderConstantBuffer> fetch( Graphics &gph,
		const CB &cb,
		unsigned slot )
	{
		return BindableMap::fetch<PixelShaderConstantBuffer>( gph,
			cb,
			slot );
	}

	static std::shared_ptr<PixelShaderConstantBuffer> fetch( Graphics &gph,
		unsigned slot )
	{
		return BindableMap::fetch<PixelShaderConstantBuffer>( gph,
			slot );
	}

	static std::string generateUid( const CB &cb,
		unsigned slot )
	{
		return generateUid( slot );
	}

	static std::string generateUid( unsigned slot )
	{
		using namespace std::string_literals;
		return typeid( PixelShaderConstantBuffer ).name() + "#"s + std::to_string( slot );
	}

	std::string getUid() const noexcept override
	{
		return generateUid( m_slot );
	}
};