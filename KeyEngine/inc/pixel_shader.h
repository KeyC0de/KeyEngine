#pragma once

#include "key_wrl.h"
#include "bindable.h"


class Graphics;

class PixelShader
	: public IBindable
{
protected:
	std::string m_path;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pPsBlob;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
public:
	PixelShader( Graphics &gfx, const std::string &filepath );

	void bind( Graphics &gfx ) cond_noex override;
	static std::shared_ptr<PixelShader> fetch( Graphics &gfx, const std::string &filepath );
	ID3DBlob* getBytecode() const noexcept;
	static std::string calcUid( const std::string &filepath );
	std::string getUid() const noexcept override;
};

//=============================================================
//	\class	PixelShaderNull
//	\author	KeyC0de
//	\date	2021/06/25 20:21
//	\brief	Use it to prevent writing any pixels ---- written as a separate class to avoid a runtime branch in PixelShader::bind()
//=============================================================
class PixelShaderNull
	: public IBindable
{
public:
	PixelShaderNull( Graphics &gfx );

	void bind( Graphics &gfx ) cond_noex override;
	static std::shared_ptr<PixelShaderNull> fetch( Graphics &gfx );
	static std::string calcUid();
	std::string getUid() const noexcept override;
};