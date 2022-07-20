#pragma once

#include <DirectXMath.h>
#include "drawable.h"


class Drawable;

class Frustum
	: public Drawable
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	Frustum( Graphics &gph, float width, float height, float nearZ, float farZ );

	void setupVertexBuffer( Graphics &gph, float width, float height, float nearZ,
		float farZ );
	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	DirectX::XMMATRIX getTransform() const noexcept override;
	DirectX::XMMATRIX getPosition() const noexcept;
	DirectX::XMMATRIX getRotation() const noexcept;
};