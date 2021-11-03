#pragma once

#include "drawable.h"
#include "bindable.h"
#include "index_buffer.h"


class Cube
	: public Drawable
{
	DirectX::XMFLOAT3 m_pos{1.0f, 1.0f, 1.0f};
	float m_roll = 0.0f;
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
public:
	Cube( Graphics& gph, const DirectX::XMFLOAT3& startingPos = {1.0f, 1.0f, 1.0f},
		float scale = 1.0f );

	void setWorldPosition( const DirectX::XMFLOAT3& pos ) noexcept;
	void setWorldRotation( float roll, float pitch, float yaw ) noexcept;
	DirectX::XMMATRIX getTransform() const noexcept override;
	void displayImguiWidgets( Graphics& gph, const char* name ) noexcept;
};