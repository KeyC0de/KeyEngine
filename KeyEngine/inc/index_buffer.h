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
	IndexBuffer( Graphics &gph, const std::vector<unsigned>& indices );
	IndexBuffer( Graphics &gph, const std::string &tag, const std::vector<unsigned>& indices );

	void bind( Graphics &gph ) cond_noex override;
	unsigned getIndexCount() const noexcept;
	static std::shared_ptr<IndexBuffer> fetch( Graphics &gph, const std::string &tag, const std::vector<unsigned>& indices );
	template<typename... TArgs>
	static std::string calcUid( const std::string &tag,
		TArgs&&... argsIgnoredHere )
	{
		using namespace std::string_literals;
		return typeid( IndexBuffer ).name() + "#"s + tag;
	}
	const std::string getUid() const noexcept override;
};