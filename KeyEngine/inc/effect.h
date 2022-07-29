#pragma once

#include <vector>
#include <memory>
#include "bindable.h"


class IEffectVisitor;
class Drawable;

namespace ren
{
	class RenderQueuePass;
	class Renderer;
}

class Effect final
{
	size_t m_renderingChannels;
	bool m_active = true;
	std::string m_targetPassName;
	ren::RenderQueuePass *m_pTargetPass = nullptr;
	std::vector<std::shared_ptr<IBindable>> m_pBindables;
public:
	Effect( size_t channels, const std::string &targetPassName,
		bool bStartActive ) noexcept;
	Effect( const Effect &rhs ) noexcept;
	Effect &operator=( const Effect &rhs ) = delete;
	Effect( Effect &&rhs ) = default;
	Effect &operator=( Effect &&rhs ) = delete;

	void addBindable( std::shared_ptr<IBindable> bindable ) noexcept;
	void render( const Drawable &drawable, size_t channels ) const noexcept;
	void bind( Graphics &gph ) const cond_noex;
	bool isEnabled() const noexcept;
	void setEnabled( bool b ) noexcept;
	const std::string &getTargetPassName() const noexcept;
	void setParentDrawable( const Drawable &parent ) noexcept;
	void accept( IEffectVisitor &ev );
	void connectPass( ren::Renderer &r );
	std::vector<std::shared_ptr<IBindable>>& getBindables() noexcept;
};