#pragma once


class Mesh;
class Graphics;
class Effect;

namespace ren
{

class Job
{
	const Mesh *m_pMesh;
	const Effect *m_pEffect;
public:
	Job( const Effect *pEffect, const Mesh *pMesh );

	//===================================================
	//	\function	run
	//	\brief  1. binds mesh
	//			2. executes draw call
	//	\date	2022/08/15 17:22
	void run( Graphics &gph ) const cond_noex;
};


}//ren