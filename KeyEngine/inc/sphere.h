#pragma once

#include "mesh.h"


class Sphere
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{1.0f, 1.0f, 1.0f};
	float m_radius = 1.0f;
public:
	Sphere( Graphics &gph, const float radius );
	Sphere( Graphics &gph, const float radius, const DirectX::XMFLOAT3 &startingPos );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	virtual const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	const DirectX::XMMATRIX calcScale() const noexcept;
	void setRadius( const float radius ) noexcept;
	float getRadius() const noexcept;
};