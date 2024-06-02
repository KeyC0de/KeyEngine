#pragma once

#include "mesh.h"
#include <variant>


class Graphics;

class Cube
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$cube";
public:
	Cube( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = "assets/models/brick_wall/brick_wall_diffuse.jpg" );
};