#pragma once

#include <DirectXMath.h>


//=============================================================
//	\class	R3ctangle
//	\author	KeyC0de
//	\date	2022/08/30 12:56
//	\brief	a R3ctangle class
//			don't rename it to Rectangle, or RECT, as it conflicts with Windows headers
//=============================================================
class R3ctangle
{
	float m_left;	// x
	float m_right;
	float m_top;	// y
	float m_bottom;
public:
	static R3ctangle makeGivenCenter( const DirectX::XMFLOAT2 &center, const float halfWidth, const float halfHeight );
public:
	R3ctangle() = default;
	R3ctangle( const float left, const float right, const float top, const float bottom );
	R3ctangle( const DirectX::XMFLOAT2 &xAndY, const DirectX::XMFLOAT2 &bottomRight );
	R3ctangle( const DirectX::XMFLOAT2 &xAndY, const float width, const float height );

	//	\function	isOverlappingWith	||	\date	2021/10/17 2:27
	//	\brief	2d rectangle collision detection formula
	bool isOverlappingWith( const R3ctangle &other ) const noexcept;
	//	\function	calcScaled	||	\date	2021/10/17 2:21
	//	\brief	gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	R3ctangle calcScaled( const float offset ) const noexcept;
	DirectX::XMFLOAT2 calcCenter() const noexcept;
	float getLeft() const noexcept;
	float& getLeft();
	float getX() const noexcept;
	float& getX();
	float getRight() const noexcept;
	float& getRight();
	float getTop() const noexcept;
	float& getTop();
	float getY() const noexcept;
	float& getY();
	float getBottom() const noexcept;
	float& getBottom();
	float getWidth() const noexcept;
	float getHeight() const noexcept;
};