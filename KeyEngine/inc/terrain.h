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
	//	\function	ctor	||	\date	2022/11/21 0:01
	//	\brief	length and width is in meters (even though the base units of the engine is cm)
	Terrain( Graphics &gfx, const float initialScale = 1.0f, const std::variant<DirectX::XMFLOAT4, std::string> &colorOrTexturePath = "assets/models/brick_wall/brick_wall_diffuse.jpg", const std::string &heightMapfilename = "", const int length = 100, const int width = 100, const int normalizeAmount = 4, const int terrainAreaUnitMultiplier = 10 );
private:
	//	\function	transformVertices	||	\date	2022/11/20 16:59
	//	\brief	transforms each vertex by a specified value
	void transformVertices( ver::VBuffer &vb, const double value ) noexcept;
};