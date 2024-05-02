#pragma once


class World
{
public:
	World() = default;

	void update( const float dt );
	void render( const float frameInterpolation );
};