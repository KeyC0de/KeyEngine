#pragma once

#include <DirectXMath.h>
#include "drawable.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class CameraWidget
	: public Drawable
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	CameraWidget( Graphics& gph );

	void setPosition( const DirectX::XMFLOAT3& pos );
	void setRotation( const DirectX::XMFLOAT3& rot );
	DirectX::XMMATRIX getTransform() const noexcept override;
	DirectX::XMMATRIX getPosition() const noexcept;
	DirectX::XMMATRIX getRotation() const noexcept;
};