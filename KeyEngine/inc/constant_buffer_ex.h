#pragma once

#include "bindable.h"
#include "dynamic_constant_buffer.h"
#include "effect_visitor.h"


class IConstantBufferEx
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pD3dCb;
	unsigned m_slot;
public:
	virtual const con::CBElement& getCBExRootLayoutElement() const noexcept = 0;
protected:
	IConstantBufferEx( Graphics &gph, unsigned slot, const con::CBElement &layoutRoot, const con::Buffer *pBuf );

	void update( Graphics &gph, const con::Buffer &buf );
};

class IVertexShaderConstantBufferEx
	: public IConstantBufferEx
{
public:
	using IConstantBufferEx::IConstantBufferEx;

	void bind( Graphics &gph ) cond_noex override;
};

class IPixelShaderConstantBufferEx
	: public IConstantBufferEx
{
public:
	using IConstantBufferEx::IConstantBufferEx;

	void bind( Graphics &gph ) cond_noex override;
};

template<class T>
class ConstantBufferEx final
	: public T
{
	static_assert( std::is_base_of_v<IConstantBufferEx, T>, "T is not IConstantBufferEx!" );

	bool m_bDirty = false;
	con::Buffer m_cb;
public:
	// empty cb
	ConstantBufferEx( Graphics &gph,
		unsigned slot,
		const con::CookedLayout &layout )
		:
		T{gph, slot, *layout.shareRootElement(), nullptr},
		m_cb{con::Buffer{layout}}
	{

	}

	ConstantBufferEx( Graphics &gph,
		unsigned slot,
		const con::Buffer &cb )
		:
		T{gph, slot, cb.getRootLayoutElement(), &cb},
		m_cb{cb}
	{

	}

	const con::CBElement& getCBExRootLayoutElement() const noexcept override
	{
		return m_cb.getRootLayoutElement();
	}

	const con::Buffer& getBuffer() const noexcept
	{
		return m_cb;
	}

	void setBuffer( const con::Buffer &cb )
	{
		m_cb.copyFrom( cb );
		m_bDirty = true;
	}

	void bind( Graphics &gph ) cond_noex override
	{
		if ( m_bDirty )
		{
			T::update( gph,
				m_cb );
			m_bDirty = false;
		}
		T::bind( gph );
	}

	void accept( IEffectVisitor &ev ) override
	{
		if ( ev.visit( m_cb ) )
		{
			m_bDirty = true;
		}
	}
};

using VertexShaderConstantBufferEx = ConstantBufferEx<IVertexShaderConstantBufferEx>;
using PixelShaderConstantBufferEx = ConstantBufferEx<IPixelShaderConstantBufferEx>;