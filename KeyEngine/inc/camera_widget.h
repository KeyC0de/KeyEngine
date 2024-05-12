#pragma once

#include "mesh.h"


class Graphics;

namespace ren
{
	class Renderer;
}

class CameraWidget
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$cam";
public:
	CameraWidget( Graphics &gfx, const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f} );
};