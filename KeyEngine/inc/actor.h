#pragma once

class Actor
{
public:
	Actor() = default;

	void update( const float dt );
	void render( const float frameInterpolation );
};