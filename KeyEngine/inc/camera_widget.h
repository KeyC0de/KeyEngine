#pragma once

#include "mesh.h"
#include <variant>


class Graphics;

class CameraWidget
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$cam_widget";
public:
	CameraWidget( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{0.2f, 0.2f, 0.6f, 1.0f} );
};