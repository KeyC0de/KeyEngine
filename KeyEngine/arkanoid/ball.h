#pragma once

#include <DirectXMath.h>


class Graphics;
struct Rect;

class Ball
{
	static constexpr inline float m_radius = 7.0f;
	DirectX::XMFLOAT2 m_pos;
	DirectX::XMFLOAT2 m_vel;
public:
	Ball( const DirectX::XMFLOAT2& position, const DirectX::XMFLOAT2& velocity );

	void update( float dt );
	void render( Graphics& gph );
	bool doWallCollision( const Rect& wall );
	//===================================================
	//	\function	reboundX, reboundY
	//	\brief  direction up = [0, 1] , down = [0, -1] , left = [0, -1] , right = [0, 1]
	//	\date	2021/10/17 3:43
	void reboundX();
	void reboundY();
	Rect getRect() const noexcept;
	DirectX::XMFLOAT2 getVelocity() const noexcept;
	DirectX::XMFLOAT2 getPosition() const noexcept;
};