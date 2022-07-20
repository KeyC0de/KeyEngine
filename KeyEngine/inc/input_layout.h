#pragma once

#include "bindable.h"
#include "dynamic_vertex_buffer.h"


class VertexShader;

class InputLayout
	: public IBindable
{
protected:
	std::string m_vertexShaderUID;
	ver::VertexLayout m_vertexLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
public:
	InputLayout( Graphics &gph, const ver::VertexLayout& layout, const VertexShader& vs );

	void bind( Graphics &gph ) cond_noex override;
	const ver::VertexLayout getLayout() const noexcept;
	static std::shared_ptr<InputLayout> fetch( Graphics &gph,
		const ver::VertexLayout& layout, const VertexShader& vs );
	static std::string generateUid( const ver::VertexLayout& layout,
		const VertexShader& vs );
	std::string getUid() const noexcept override;
};