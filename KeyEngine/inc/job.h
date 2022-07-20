#pragma once


class Drawable;
class Graphics;
class Effect;

namespace ren
{

class Job final
{
	const class Drawable *m_pDrawable;
	const class Effect *m_pEffect;
public:
	Job( const Effect *pEffect, const Drawable *pDrawable );

	// 1. binds drawable, 2. draw call
	void run( Graphics &gph ) const cond_noex;
};


}//ren