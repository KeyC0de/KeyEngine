#pragma once

#include "mesh.h"


class Cube
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{1.0f, 1.0f, 1.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	Cube( Graphics &gph, const DirectX::XMFLOAT3 &startingPos = {1.0f, 1.0f, 1.0f}, const float scale = 1.0f );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	virtual const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	const DirectX::XMMATRIX calcRotation() const noexcept;
	void displayImguiWidgets( Graphics &gph, const char *name ) noexcept;
};