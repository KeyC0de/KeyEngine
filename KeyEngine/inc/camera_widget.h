#pragma once

#include <DirectXMath.h>
#include "mesh.h"


class Graphics;

namespace ren
{
	class Renderer;
}

// #TODO: get rid of m_pos and m_rot, set everything with 1 transform matrix of the base Mesh
class CameraWidget
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	CameraWidget( Graphics &gph );

	void setPosition( const DirectX::XMFLOAT3 &pos );
	void setRotation( const DirectX::XMFLOAT3 &rot );
	const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX getPosition() const noexcept;
	const DirectX::XMMATRIX getRotation() const noexcept;
};