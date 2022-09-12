#pragma once

#include <DirectXMath.h>
#include "mesh.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class CameraWidget
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	CameraWidget( Graphics &gph );

	void setPosition( const DirectX::XMFLOAT3 &pos );
	void setRotation( const DirectX::XMFLOAT3 &rot );
	virtual const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	const DirectX::XMMATRIX calcRotation() const noexcept;
};