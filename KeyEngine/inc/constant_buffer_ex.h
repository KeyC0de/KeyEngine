#pragma once

#include "key_wrl.h"
#include "bindable.h"
#include "dynamic_constant_buffer.h"
#ifndef FINAL_RELEASE
#	include "imgui_visitors.h"
#endif


class Graphics;

class IConstantBufferEx
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pD3dCb;
	unsigned m_slot;
protected:
	static void setBufferDesc( D3D11_BUFFER_DESC &d3dBufDesc, const unsigned byteWidth );
protected:
	IConstantBufferEx( Graphics &gfx, const unsigned slot, const con::CBElement &layoutRoot, const con::CBuffer *pBuf );

	void update( Graphics &gfx, const con::CBuffer &buf );
public:
	virtual const con::CBElement& getCbRootElement() const noexcept = 0;
};

class IVertexShaderConstantBufferEx
	: public IConstantBufferEx
{
public:
	using IConstantBufferEx::IConstantBufferEx;

	void bind( Graphics &gfx ) cond_noex override;
};

class IPixelShaderConstantBufferEx
	: public IConstantBufferEx
{
public:
	using IConstantBufferEx::IConstantBufferEx;

	void bind( Graphics &gfx ) cond_noex override;
};

template<class T>
class ConstantBufferEx final
	: public T
{
	static_assert( std::is_base_of_v<IConstantBufferEx, T>, "T is not IConstantBufferEx!" );

	bool m_bDirty = false;
	con::CBuffer m_cb;
public:
	// empty cb
	ConstantBufferEx( Graphics &gfx,
		const unsigned slot,
		const con::CookedLayout &layout )
		:
		T{gfx, slot, *layout.shareRootElement(), nullptr},
		m_cb{con::CBuffer{layout}}
	{

	}

	ConstantBufferEx( Graphics &gfx,
		const unsigned slot,
		const con::CBuffer &cb )
		:
		T{gfx, slot, cb.getRootElement(), &cb},
		m_cb{cb}
	{

	}

	const con::CBElement& getCbRootElement() const noexcept override
	{
		return m_cb.getRootElement();
	}

	con::CBuffer getBufferCopy() const noexcept
	{
		return m_cb;
	}

	const con::CBuffer& getBuffer() const noexcept
	{
		return m_cb;
	}

	void setBuffer( const con::CBuffer &cb )
	{
		m_cb.copyFrom( cb );
		m_bDirty = true;
	}

	void bind( Graphics &gfx ) cond_noex override
	{
		if ( m_bDirty )
		{
			T::update( gfx, m_cb );
			m_bDirty = false;
		}
		T::bind( gfx );
	}

	void accept( IImGuiConstantBufferVisitor &ev ) override
	{
		if ( ev.visit( m_cb ) )
		{
			m_bDirty = true;
		}
	}
};

using VertexShaderConstantBufferEx = ConstantBufferEx<IVertexShaderConstantBufferEx>;
using PixelShaderConstantBufferEx = ConstantBufferEx<IPixelShaderConstantBufferEx>;