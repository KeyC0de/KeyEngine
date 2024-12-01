#pragma once

#include "render_queue_pass.h"


class Graphics;
class Camera;

namespace ren
{

///=============================================================
/// \class	TransparentPass
/// \author	KeyC0de
/// \date	2024/05/05 14:42
/// \brief	A pass for transparent meshes
///=============================================================
class TransparentPass
	: public RenderQueuePass
{
	const Camera *m_pActiveCamera = nullptr;
public:
	/// \brief	TextureSamplerState is set per mesh depending on whether the mesh has a texture
	/// \brief	RasterizerState is set per mesh depending on whether the mesh's texture has an alpha channel
	/// \brief	by default you Cull_Back like Opaque meshes; or use CULL_NONE for double sided transparency (may be useful for very thin meshes, like leaves, or billboards)
	/// \brief	BlendState is set per mesh depending on the level of transparency of the mesh (determined by its flat color's Alpha value or itstexture alpha value)
	TransparentPass( Graphics &gfx, const std::string &name );

	void run( Graphics &gfx ) const cond_noex override;
	void setActiveCamera( const Camera &cam ) noexcept;
};


}