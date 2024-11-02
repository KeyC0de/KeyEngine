#pragma once

#include <vector>
#include <memory>
#include "bindable.h"


class Mesh;
class IImGuiConstantBufferVisitor;

namespace ren
{
class RenderQueuePass;
class Renderer;
}

class Material
{
	size_t m_renderingChannels;
	bool m_bActive = true;
	std::string m_targetPassName;
	ren::RenderQueuePass *m_pTargetPass = nullptr;
	std::vector<std::shared_ptr<IBindable>> m_bindables;
public:
	//	\function	Material	||	\date	2024/04/25 13:55
	//	\brief	the channels is a bitwise mask (rendering_channel.h) which corresponds to the Rendering channel(s) used for this Material
	//			the rendering channel can cross multiple `Pass`es to render, but a single `Material` targets a single `Pass`
	//			so channels are used to group similar materials
	//			bStartActive refers to whether this Material is enabled (not the Pass or the channel(s)), if the Material is not enabled at a specific frame it will not pass its bindables to the renderer and as such the Mesh won't render the material
	Material( const size_t channels, const std::string &targetPassName, const bool bStartActive ) noexcept;
	Material( const Material &rhs );
	Material& operator=( const Material &rhs ) = delete;
	Material( Material &&rhs ) noexcept;
	Material& operator=( Material &&rhs ) = delete;

	void addBindable( std::shared_ptr<IBindable> pBindable ) noexcept;
	void render( const Mesh &mesh, const size_t channels ) const noexcept;
	void bind( Graphics &gfx ) const cond_noex;
	bool isEnabled() const noexcept;
	void setEnabled( const bool b ) noexcept;
	void setEnabled( const size_t channels, const bool bEnabled ) noexcept;
	const std::string& getTargetPassName() const noexcept;
	void setMesh( const Mesh &parent ) noexcept;
	void accept( IImGuiConstantBufferVisitor &ev );
	void connectPass( ren::Renderer &r );
	std::vector<std::shared_ptr<IBindable>>& getBindables();
	const std::vector<std::shared_ptr<IBindable>>& getBindables() const noexcept;
	size_t getChannelMask() const noexcept;
};