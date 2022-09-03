#pragma once

#include <DirectXMath.h>
#include "mesh.h"


class Mesh;

class Frustum
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
public:
	Frustum( Graphics &gph, const float width, const float height, const float nearZ, const float farZ );

	void setupVertexBuffer( Graphics &gph, const float width, const float height, const float nearZ, const float farZ );
	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcRotation() const noexcept;
	const DirectX::XMMATRIX calcPosition() const noexcept;
};