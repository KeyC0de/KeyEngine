#pragma once

#include <variant>
#include <DirectXMath.h>
#include "mesh.h"


class Graphics;

class Terrain
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$terrainGrid";
	int m_quadWidth = 0;	// #TODO:
	int m_quadHeight = 0;	// #TODO:
public:
	/// \brief	length and width is in meters (even though the base units of the engine is cm)
	Terrain( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = "assets/models/brick_wall/brick_wall_diffuse.jpg", const std::string &heightMapfilename = "", const int length = 100, const int width = 100, const int normalizeAmount = 4, const int terrainAreaUnitMultiplier = 10 );
private:
	/// \brief	transforms each vertex's position by the specified matrix
	void transformVerticesPosition( ver::VBuffer &vb, const DirectX::XMMATRIX &matrix ) noexcept;
};