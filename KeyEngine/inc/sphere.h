#pragma once

#include "drawable.h"


class Sphere
	: public Drawable
{
	DirectX::XMFLOAT3 m_pos{1.0f, 1.0f, 1.0f};
public:
	Sphere( Graphics &gph, float radius );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	DirectX::XMMATRIX getTransform() const noexcept override;
};