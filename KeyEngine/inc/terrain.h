#pragma once

#include "mesh.h"


class Terrain
	: public Mesh
{
	DirectX::XMFLOAT3 m_pos{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT3 m_rot{0.0f, 0.0f, 0.0f};
	int m_quadWidth = 0;
	int m_quadHeight = 0;
public:
	//	\function	ctor	||	\date	2022/11/21 0:01
	//	\brief	length and width is in meters (even though the base units of the engine is cm)
	Terrain( Graphics &gph, const int length, const int width, const std::string &heightMapfilename = "", const int normalizeAmount = 4, const int terrainAreaUnitMultiplier = 10, const DirectX::XMFLOAT3 &startingPos = {0.0f, 0.0f, 0.0f} );

	void setPosition( const DirectX::XMFLOAT3 &pos ) noexcept;
	void setRotation( const DirectX::XMFLOAT3 &rot ) noexcept;
	virtual const DirectX::XMMATRIX getTransform() const noexcept override;
	const DirectX::XMMATRIX calcPosition() const noexcept;
	const DirectX::XMMATRIX calcRotation() const noexcept;
	void displayImguiWidgets( Graphics &gph, const char *name ) noexcept;
	//	\function	transformVertices	||	\date	2022/11/20 16:59
	//	\brief	transforms each vertex by a specified value - currently UNUSED
	void transformVertices( ver::VBuffer &vb, const double value ) noexcept;
private:
	// #TODO: rework this put them in Mesh - not in Model
	DirectX::XMMATRIX getRotation( const DirectX::XMFLOAT3 &rotIn ) cond_noex;
};