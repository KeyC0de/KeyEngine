#pragma once

#include "mesh.h"


class Sphere
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$sphere";
	float m_radius = 1.0f;
public:
	Sphere( Graphics &gfx, const float radius = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f} );

	void setRadius( const float radius ) noexcept;
	float getRadius() const noexcept;
};