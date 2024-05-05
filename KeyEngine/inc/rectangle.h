#pragma once

#include <DirectXMath.h>


//=============================================================
//	\class	Rect
//	\author	KeyC0de
//	\date	2022/08/30 12:56
//	\brief	a Rectangle class
//			don't rename it to Rectangle, as it conflicts with Windows headers
//=============================================================
struct Rect
{
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;

	static Rect makeGivenCenter( const DirectX::XMFLOAT2 &center, const float halfWidth, const float halfHeight );

	Rect() = default;
	Rect( const float left, const float right, const float top, const float bottom );
	Rect( const DirectX::XMFLOAT2 &topLeft, const DirectX::XMFLOAT2 &bottomRight );
	Rect( const DirectX::XMFLOAT2 &topLeft, const float width, const float height );

	//	\function	isOverlappingWith	||	\date	2021/10/17 2:27
	//	\brief	2d rectangle collision detection formula
	bool isOverlappingWith( const Rect &other ) const noexcept;
	//	\function	calcScaled	||	\date	2021/10/17 2:21
	//	\brief	gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	Rect calcScaled( const float offset ) const noexcept;
	float getWidth() const noexcept;
	float getHeight() const noexcept;
	DirectX::XMFLOAT2 calcCenter() const noexcept;
};