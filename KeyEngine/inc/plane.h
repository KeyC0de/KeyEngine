#pragma once

#include "mesh.h"


class Plane
	: public Mesh
{
	struct ColorPCB
	{
		DirectX::XMFLOAT4 color;
	};
	ColorPCB m_colPcb;
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	float m_roll = 0.0f;
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
public:
	Plane( Graphics &gph, const float size, const DirectX::XMFLOAT4 &color = {1.0f, 1.0f, 1.0f, 1.0f} );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const float roll, const float pitch, const float yaw ) noexcept;
	const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcRotation() const noexcept;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	void displayImguiWidgets( Graphics &gph, const std::string &name ) noexcept;
};