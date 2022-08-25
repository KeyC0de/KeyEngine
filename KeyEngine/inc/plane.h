#pragma once

#include "drawable.h"


class Plane
	: public Drawable
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
	Plane( Graphics &gph, float size,
		const DirectX::XMFLOAT4 &color = {1.0f, 1.0f, 1.0f, 1.0f} );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( float roll, float pitch, float yaw ) noexcept;
	DirectX::XMMATRIX getTransform() const noexcept override;
	void displayImguiWidgets( Graphics &gph, const std::string &name ) noexcept;
};