#pragma once

#include "mesh.h"


class Line
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$line";
	struct ColorPSCB
	{
		DirectX::XMFLOAT4 materialColor;
	} m_colorPscb;
public:
	Line( Graphics &gfx, const float length = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT4 &color = {1.0f, 0.4f, 0.4f, 1.0f} );

	virtual void displayImguiWidgets( Graphics &gfx, const std::string &name ) noexcept override;
};