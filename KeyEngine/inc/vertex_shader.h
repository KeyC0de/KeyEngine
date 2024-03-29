#pragma once

#include "bindable.h"


class VertexShader
	: public IBindable
{
	std::string m_path;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pVsBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
public:
	VertexShader( Graphics &gph, const std::string &filepath );

	void bind( Graphics &gph ) cond_noex override;
	ID3DBlob* getBytecode() const noexcept;
	static std::shared_ptr<VertexShader> fetch( Graphics &gph, const std::string &filepath );
	static std::string calcUid( const std::string &filepath );
	const std::string getUid() const noexcept override;
};