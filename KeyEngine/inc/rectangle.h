#pragma once

#include <DirectXMath.h>
#include "winner.h"


//=============================================================
//	\class	RectangleF
//	\author	KeyC0de
//	\date	2022/08/30 12:56
//	\brief	a Rectangle class with floats
//			don't rename it to Rectangle, or RECT, as it conflicts with Windows headers
//=============================================================
class RectangleF
{
	float m_left;	// x
	float m_right;
	float m_top;	// y
	float m_bottom;
public:
	static RectangleF makeGivenCenter( const DirectX::XMFLOAT2 &center, const float halfWidth, const float halfHeight );
public:
	RectangleF() = default;
	RectangleF( const int x, const int y, const int width, const int height );
	RectangleF( const float left, const float right, const float top, const float bottom );
	RectangleF( const DirectX::XMFLOAT2 &xAndY, const DirectX::XMFLOAT2 &bottomRight );
	RectangleF( const DirectX::XMFLOAT2 &xAndY, const float width, const float height );

	operator RECT()
	{
		return {static_cast<long>( m_left ), static_cast<long>( m_top ), static_cast<long>( m_right ), static_cast<long>( m_bottom )};
	}

	//	\function	isOverlappingWith	||	\date	2021/10/17 2:27
	//	\brief	2d rectangle collision detection formula
	bool isOverlappingWith( const RectangleF &other ) const noexcept;
	//	\function	calcScaled	||	\date	2021/10/17 2:21
	//	\brief	gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	RectangleF calcScaled( const float offset ) const noexcept;
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

//=============================================================
//	\class	RectangleI
//	\author	KeyC0de
//	\date	2022/08/30 12:56
//	\brief	a Rectangle class with ints
//			don't rename it to Rectangle, or RECT, as it conflicts with Windows headers
//=============================================================
class RectangleI
{
	int m_left;	// x
	int m_right;
	int m_top;	// y
	int m_bottom;
public:
	static RectangleI makeGivenCenter( const DirectX::XMFLOAT2 &center, const int halfWidth, const int halfHeight );
public:
	RectangleI() = default;
	RectangleI( const int x, const int y, const int width, const int height );

	operator RECT()
	{
		return {static_cast<long>( m_left ), static_cast<long>( m_top ), static_cast<long>( m_right ), static_cast<long>( m_bottom )};
	}

	//	\function	isOverlappingWith	||	\date	2021/10/17 2:27
	//	\brief	2d rectangle collision detection formula
	bool isOverlappingWith( const RectangleI &other ) const noexcept;
	//	\function	calcScaled	||	\date	2021/10/17 2:21
	//	\brief	gets an expanded (offset>0) or minimized (offset<0) form of the rectangle
	RectangleI calcScaled( const int offset ) const noexcept;
	DirectX::XMFLOAT2 calcCenter() const noexcept;
	int getLeft() const noexcept;
	int& getLeft();
	int getX() const noexcept;
	int& getX();
	int getRight() const noexcept;
	int& getRight();
	int getTop() const noexcept;
	int& getTop();
	int getY() const noexcept;
	int& getY();
	int getBottom() const noexcept;
	int& getBottom();
	int getWidth() const noexcept;
	int getHeight() const noexcept;
};