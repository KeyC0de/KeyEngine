#pragma once

#include "bindable.h"


class PixelShader
	: public IBindable
{
protected:
	std::string m_path;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pPsBlob;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
public:
	PixelShader( Graphics& gph, const std::string& filepath );

	void bind( Graphics& gph ) cond_noex override;
	static std::shared_ptr<PixelShader> fetch( Graphics& gph, const std::string& filepath );
	ID3DBlob* getBytecode() const noexcept;
	static std::string generateUid( const std::string& filepath );
	std::string getUid() const noexcept override;
};