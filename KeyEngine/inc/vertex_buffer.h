#pragma once

#include <string>
#include "key_wrl.h"
#include "bindable.h"
#include "dynamic_vertex_buffer.h"


class Graphics;

class VertexBuffer
	: public IBindable
{
	unsigned m_stride;
	std::string m_tag;
	ver::VertexInputLayout m_vertexLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
public:
	VertexBuffer( Graphics &gfx, const ver::VBuffer &vb );
	VertexBuffer( Graphics &gfx, const std::string &tag, const ver::VBuffer &vb );

	void bind( Graphics &gfx ) cond_noex override;
	const ver::VertexInputLayout& getLayout() const noexcept;
	static std::shared_ptr<VertexBuffer> fetch( Graphics &gfx, const std::string &tag, const ver::VBuffer &vb );
	template<typename... TArgsIgnored>
	static std::string calcUid( const std::string &tag,
		TArgsIgnored &&... )
	{
		using namespace std::string_literals;
		return typeid( VertexBuffer ).name() + "#"s + tag;
	}
	std::string getUid() const noexcept override;
};