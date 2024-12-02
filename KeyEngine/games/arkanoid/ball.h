#pragma once

#include <DirectXMath.h>


class Graphics;
class RectangleF;

class Ball
{
	static constexpr inline float s_radius = 7.0f;
	DirectX::XMFLOAT2 m_pos;
	DirectX::XMFLOAT2 m_vel;
public:
	Ball( const DirectX::XMFLOAT2 &position, const DirectX::XMFLOAT2 &velocity );

	void update( const float dt );
	void render( Graphics &gfx ) cond_noex;
	bool doWallCollision( const RectangleF &wall );
	/// \brief	direction up = [0, 1] , down = [0, -1] , left = [0, -1] , right = [0, 1]
	void reboundX();
	void reboundY();
	RectangleF rect() const noexcept;
	const DirectX::XMFLOAT2& getVelocity() const noexcept;
	const DirectX::XMFLOAT2& getPosition() const noexcept;
};