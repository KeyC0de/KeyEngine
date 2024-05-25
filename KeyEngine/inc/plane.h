#pragma once

#include "mesh.h"


class Plane
	: public Mesh
{
	struct ColorPSCB
	{
		DirectX::XMFLOAT4 materialColor;
	} m_colPcb;
public:
	Plane( Graphics &gfx, const int length = 2, const int width = 2, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT4 &color = {1.0f, 1.0f, 1.0f, 1.0f}, const std::string &diffuseTexturePath = "assets/models/brick_wall/brick_wall_diffuse.jpg" );

	virtual void displayImguiWidgets( Graphics &gfx, const std::string &name ) noexcept override;
};