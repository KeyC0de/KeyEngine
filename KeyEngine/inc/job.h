#pragma once


class Drawable;
class Graphics;
class Effect;

namespace ren
{

class Job
{
	const Drawable *m_pDrawable;
	const Effect *m_pEffect;
public:
	Job( const Effect *pEffect, const Drawable *pDrawable );

	//===================================================
	//	\function	run
	//	\brief  1. binds drawable
	//			2. executes draw call
	//	\date	2022/08/15 17:22
	void run( Graphics &gph ) const cond_noex;
};


}//ren