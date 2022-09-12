#pragma once

#include "mesh.h"


class Plane
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
	
	struct ColorPCB
	{
		DirectX::XMFLOAT4 color;
	} m_colPcb;
public:
	Plane( Graphics &gph, const DirectX::XMFLOAT4 &color = {1.0f, 1.0f, 1.0f, 1.0f}, const float scale = 1.0f );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	virtual const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	const DirectX::XMMATRIX calcRotation() const noexcept;
	void displayImguiWidgets( Graphics &gph, const std::string &name ) noexcept;
};