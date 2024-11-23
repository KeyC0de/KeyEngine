#pragma once

#include "key_wrl.h"
#include <array>
#include "bindable.h"
#include "texture_desc.h"


class Graphics;

class RasterizerState
	: public IBindable
{
public:
	enum RasterizerMode
	{
		DefaultRS,
		ShadowRS,
	};
	enum FillMode
	{
		Solid,
		Wireframe,
	};
	enum FaceMode
	{
		Front,	// cull backsided faces (default)
		Back,	// cull frontsided faces
		Both,	// don't cull any faces
	};
protected:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	RasterizerMode m_mode;
	FillMode m_fillMode;
	FaceMode m_faceMode;
	int m_depthBias;
	float m_slopeBias;
	float m_biasClamp;
public:
	/// \brief	if mode is RasterizerMode::ShadowRS then fillMode and faceMode values will be defaulted
	/// \brief	if mode is RasterizerMode::DefaultRS then depthBias, slopeBias and biasClamp will be defaulted
	RasterizerState( Graphics &gfx, const RasterizerMode mode, const FillMode fillMode, const FaceMode faceMode, const int depthBias = D3D11_DEFAULT_DEPTH_BIAS, const float slopeBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, const float biasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP );

	void bind( Graphics &gfx ) cond_noex override;
	static std::shared_ptr<RasterizerState> fetch( Graphics &gfx, const RasterizerMode mode, const FillMode fillMode, const FaceMode faceMode, const int depthBias = D3D11_DEFAULT_DEPTH_BIAS, const float slopeBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, const float biasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP );
	static std::string calcUid( const RasterizerMode mode, const FillMode fillMode, const FaceMode faceMode, const int depthBias = D3D11_DEFAULT_DEPTH_BIAS, const float slopeBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, const float biasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP );
	std::string getUid() const noexcept override;
	RasterizerMode getMode() const noexcept;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>& getD3dRasterizerState();
};