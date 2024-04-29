#pragma once

#include <string>
#include "bindable.h"
#include "dynamic_vertex_buffer.h"


class VertexBuffer
	: public IBindable
{
	unsigned m_stride;
	std::string m_tag;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ver::VertexInputLayout m_vertexLayout;
public:
	VertexBuffer( Graphics &gph, const std::string &tag, const ver::VBuffer &vb );
	VertexBuffer( Graphics &gph, const ver::VBuffer &vb );

	void bind( Graphics &gph ) cond_noex override;
	const ver::VertexInputLayout& getLayout() const noexcept;
	static std::shared_ptr<VertexBuffer> fetch( Graphics &gph, const std::string &tag, const ver::VBuffer &vb );
	template<typename... TArgsIgnored>
	static std::string calcUid( const std::string &tag,
		TArgsIgnored &&... )
	{
		using namespace std::string_literals;
		return typeid( VertexBuffer ).name() + "#"s + tag;
	}
	std::string getUid() const noexcept override;
};