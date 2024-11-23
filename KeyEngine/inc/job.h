#pragma once


class Mesh;
class Graphics;
class Material;

namespace ren
{

class Job
{
	const Mesh *m_pMesh;
	const Material *m_pMaterial;
public:
	Job( const Mesh *pMesh, const Material *pMaterial );

	/// \brief	1. binds mesh
	/// \brief	2. executes draw call
	void run( Graphics &gfx ) const cond_noex;
};


}//ren