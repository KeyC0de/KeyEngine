#pragma once

#include "mesh.h"
#include <variant>


class Graphics;

class Sphere
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$sphere";
public:
	Sphere( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f} );
};