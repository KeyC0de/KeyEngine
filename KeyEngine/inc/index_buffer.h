#pragma once

#include <string>
#include "bindable.h"


class IndexBuffer
	: public IBindable
{
	std::string m_tag;
	unsigned m_count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
public:
	IndexBuffer( Graphics &gfx, const std::vector<unsigned> &indices );
	IndexBuffer( Graphics &gfx, const std::string &tag, const std::vector<unsigned> &indices );

	void bind( Graphics &gfx ) cond_noex override;
	unsigned getIndexCount() const noexcept;
	static std::shared_ptr<IndexBuffer> fetch( Graphics &gfx, const std::string &tag, const std::vector<unsigned> &indices );
	template<typename ...TArgsIgnored>
	static std::string calcUid( const std::string &tag,
		TArgsIgnored &&... )
	{
		using namespace std::string_literals;
		return typeid( IndexBuffer ).name() + "#"s + tag;
	}
	std::string getUid() const noexcept override;
};