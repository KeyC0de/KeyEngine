#pragma once

#include "mesh.h"
#include "bindable.h"
#include "index_buffer.h"


class Cube
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{1.0f, 1.0f, 1.0f};
	float m_roll = 0.0f;
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
public:
	Cube( Graphics &gph, const DirectX::XMFLOAT3 &startingPos = {1.0f, 1.0f, 1.0f}, const float scale = 1.0f );

	void setWorldPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setWorldRotation( const float roll, const float pitch, const float yaw ) noexcept;
	const DirectX::XMMATRIX getTransform() const noexcept override;
	void displayImguiWidgets( Graphics &gph, const char *name ) noexcept;
};