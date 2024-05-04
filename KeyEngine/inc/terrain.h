#pragma once

#include "mesh.h"


class Terrain
	: public Mesh
{
	static inline constexpr const char *s_geometryTag = "$terrainGrid";
	int m_quadWidth = 0;	// #TODO:
	int m_quadHeight = 0;	// #TODO:
public:
	//	\function	ctor	||	\date	2022/11/21 0:01
	//	\brief	length and width is in meters (even though the base units of the engine is cm)
	Terrain( Graphics &gfx, const int length, const int width, const std::string &heightMapfilename = "", const float initialScale = 1.0f, const DirectX::XMFLOAT3 &initialRot = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3 &initialPos = {0.0f, 0.0f, 0.0f}, const int normalizeAmount = 4, const int terrainAreaUnitMultiplier = 10 );

	virtual void displayImguiWidgets( Graphics &gfx, const std::string &name ) noexcept override;
private:
	//	\function	transformVertices	||	\date	2022/11/20 16:59
	//	\brief	transforms each vertex by a specified value - currently UNUSED
	void transformVertices( ver::VBuffer &vb, const double value ) noexcept;
};