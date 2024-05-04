#pragma once

#include "bindable.h"
#include "dynamic_vertex_buffer.h"


class VertexShader;

//=============================================================
//	\class	InputLayout
//	\author	KeyC0de
//	\date	2022/08/30 1:16
//	\brief	Vertex Input Layout bindable
//=============================================================
class InputLayout
	: public IBindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
	std::string m_vertexShaderUID;
	ver::VertexInputLayout m_vertexLayout;
public:
	InputLayout( Graphics &gfx, const ver::VertexInputLayout &layout, const VertexShader &vs );

	void bind( Graphics &gfx ) cond_noex override;
	const ver::VertexInputLayout getLayout() const noexcept;
	static std::shared_ptr<InputLayout> fetch( Graphics &gfx, const ver::VertexInputLayout &layout, const VertexShader &vs );
	static std::string calcUid( const ver::VertexInputLayout &layout, const VertexShader &vs );
	std::string getUid() const noexcept override;
};