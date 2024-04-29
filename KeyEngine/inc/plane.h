#pragma once

#include "mesh.h"


class Plane
	: public Mesh
{
	struct ColorPCB
	{
		DirectX::XMFLOAT3 color;
		float padding = 0.0f;
	} m_colPcb;
public:
	Plane( Graphics &gph, const int length = 2, const int width = 2, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &color = {1.0f, 1.0f, 1.0f} );

	virtual void displayImguiWidgets( Graphics &gph, const std::string &name ) noexcept override;
};