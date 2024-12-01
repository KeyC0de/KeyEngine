#pragma once

#include "mesh.h"
#include <variant>


class Graphics;

class CameraFrustum
	: public Mesh
{
	static inline constexpr const char* s_geometryTag = "$cam_frustum";
public:
	CameraFrustum( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{0.6f, 0.2f, 0.2f, 1.0f}, const float width = 1.0f, const float height = 1.0f, const float nearZ = 0.5f, const float farZ = 200.0f, const float fovDegrees = 90.0f );
};