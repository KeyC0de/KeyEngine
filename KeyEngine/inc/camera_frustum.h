#pragma once

#include "mesh.h"


class CameraFrustum
	: public Mesh
{
	static inline constexpr const char* s_geometryTag = "$frustum";
public:
	CameraFrustum( Graphics &gfx, const float width, const float height, const float nearZ, const float farZ, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f} );

	std::shared_ptr<VertexBuffer>& getVertexBuffer();
};