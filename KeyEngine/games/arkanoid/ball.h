#pragma once

#include <DirectXMath.h>


class Graphics;
struct Rect;

class Ball
{
	static constexpr inline float s_radius = 7.0f;
	DirectX::XMFLOAT2 m_pos;
	DirectX::XMFLOAT2 m_vel;
public:
	Ball( const DirectX::XMFLOAT2 &position, const DirectX::XMFLOAT2 &velocity );

	void update( const float dt );
	void render( Graphics &gfx ) cond_noex;
	bool doWallCollision( const Rect &wall );
	//===================================================
	//	\function	reboundX, reboundY	||	\date	2021/10/17 3:43
	//	\brief	direction up = [0, 1] , down = [0, -1] , left = [0, -1] , right = [0, 1]
	void reboundX();
	void reboundY();
	Rect rect() const noexcept;
	const DirectX::XMFLOAT2& getVelocity() const noexcept;
	const DirectX::XMFLOAT2& getPosition() const noexcept;
};