#pragma once

#include <DirectXMath.h>


//=============================================================
//	\class	Rect
//	\author	KeyC0de
//	\date	2022/08/30 12:56
//	\brief	a R3ctangle class
//			don't rename it to R3ctangle, as it conflicts with Windows headers
//=============================================================
struct R3ctangle
{
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;

	static R3ctangle makeGivenCenter( const DirectX::XMFLOAT2 &center, const float halfWidth, const float halfHeight );

	R3ctangle() = default;
	R3ctangle( const float left, const float right, const float top, const float bottom );
	R3ctangle( const DirectX::XMFLOAT2 &topLeft, const DirectX::XMFLOAT2 &bottomRight );
	R3ctangle( const DirectX::XMFLOAT2 &topLeft, const float width, const float height );

	//	\function	isOverlappingWith	||	\date	2021/10/17 2:27
	//	\brief	2d rectangle collision detection formula
	bool isOverlappingWith( const R3ctangle &other ) const noexcept;
	//	\function	calcScaled	||	\date	2021/10/17 2:21
	//	\brief	gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	R3ctangle calcScaled( const float offset ) const noexcept;
	float getWidth() const noexcept;
	float getHeight() const noexcept;
	DirectX::XMFLOAT2 calcCenter() const noexcept;
};