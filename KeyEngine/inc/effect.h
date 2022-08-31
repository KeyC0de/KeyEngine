#pragma once

#include <vector>
#include <memory>
#include "bindable.h"


class Mesh;
class IEffectVisitor;

namespace ren
{
	class RenderQueuePass;
	class Renderer;
}

class Effect
{
	size_t m_renderingChannels;
	bool m_active = true;
	std::string m_targetPassName;
	ren::RenderQueuePass *m_pTargetPass = nullptr;
	std::vector<std::shared_ptr<IBindable>> m_bindables;
public:
	Effect( size_t channels, const std::string &targetPassName, const bool bStartActive ) noexcept;
	Effect( const Effect &rhs );
	Effect& operator=( const Effect &rhs ) = delete;
	Effect( Effect &&rhs ) noexcept;
	Effect& operator=( Effect &&rhs ) = delete;

	void addBindable( std::shared_ptr<IBindable> pBindable ) noexcept;
	void render( const Mesh &mesh, const size_t channels ) const noexcept;
	void bind( Graphics &gph ) const cond_noex;
	bool isEnabled() const noexcept;
	void setEnabled( const bool b ) noexcept;
	const std::string& getTargetPassName() const noexcept;
	void setMesh( const Mesh &parent ) noexcept;
	void accept( IEffectVisitor &ev );
	void connectPass( ren::Renderer &r );
	std::vector<std::shared_ptr<IBindable>>& bindables() noexcept;
	const std::vector<std::shared_ptr<IBindable>>& getBindables() const noexcept;
};