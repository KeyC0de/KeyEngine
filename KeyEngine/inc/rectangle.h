#pragma once

#include <DirectXMath.h>


struct Rect
{
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;
	
	static Rect makeGivenCenter( const DirectX::XMFLOAT2 &center, float halfWidth,
		float halfHeight );

	Rect() = default;
	Rect( float left, float right, float top, float bottom );
	Rect( const DirectX::XMFLOAT2 &topLeft, const DirectX::XMFLOAT2 &bottomRight );
	Rect( const DirectX::XMFLOAT2 &topLeft, float width, float height );
	
	//===================================================
	//	\function	isOverlappingWith
	//	\brief  2d rectangle collision detection formula
	//	\date	2021/10/17 2:27
	bool isOverlappingWith( const Rect &other ) const noexcept;
	//===================================================
	//	\function	getScaled
	//	\brief  gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	//	\date	2021/10/17 2:21
	Rect getScaled( float offset ) const noexcept;
	float getRectWidth() const noexcept;
	float getRectHeight() const noexcept;
	DirectX::XMFLOAT2 getCenter() const noexcept;
};