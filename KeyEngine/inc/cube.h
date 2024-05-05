#pragma once

#include "mesh.h"


class Cube
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$cube";
	struct ColorPCB
	{
		DirectX::XMFLOAT4 color;
	} m_colPcb;
public:
	Cube( Graphics &gfx, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT4 &color = {1.0f, 0.4f, 0.4f, 1.0f} );

	virtual void displayImguiWidgets( Graphics &gfx, const std::string &name ) noexcept override;
};