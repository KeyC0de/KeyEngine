#pragma once

#include <string>
#include "bindable.h"
#include "dynamic_vertex_buffer.h"


class VertexBuffer
	: public IBindable
{
	std::string m_tag;
	unsigned m_stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ver::VertexLayout m_vertexLayout;
public:
	VertexBuffer( Graphics &gph, const std::string &tag, const ver::Buffer& vb );
	VertexBuffer( Graphics &gph, const ver::Buffer& vb );

	void bind( Graphics &gph ) cond_noex override;
	const ver::VertexLayout& getLayout() const noexcept;
	static std::shared_ptr<VertexBuffer> fetch( Graphics &gph, const std::string &tag,
		const ver::Buffer& vb );
	template<typename... TArgsIgnored>
	static std::string generateUid( const std::string &tag,
		TArgsIgnored&&... )
	{
		using namespace std::string_literals;
		return typeid( VertexBuffer ).name() + "#"s + tag;
	}
	std::string getUid() const noexcept override;
};