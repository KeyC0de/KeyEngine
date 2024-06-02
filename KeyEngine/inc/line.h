#pragma once

#include "mesh.h"
#include <variant>


class Graphics;

class Line
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$line";
public:
	Line( Graphics &gfx, const float lengthScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = DirectX::XMFLOAT4{1.0f, 0.4f, 0.4f, 1.0f} );
};