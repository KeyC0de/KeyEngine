#pragma once

#include <memory>
#include "pass.h"
#include "consumer.h"


class IBindable;
class IRenderTargetView;
class IDepthStencilView;

namespace ren
{

class IBindablePass
	: public IPass
{
	std::vector<std::shared_ptr<IBindable>> m_bindables;
protected:
	// targets of the current pass (either back-buffer or offscreen target):
	std::shared_ptr<IRenderTargetView> m_pRtv;
	std::shared_ptr<IDepthStencilView> m_pDsv;
protected:
	IBindablePass( const std::string &name,
		const std::vector<std::shared_ptr<IBindable>>& bindables = {} );

	//===================================================
	//	\function	addPassBindable
	//	\brief  add Bindables that will be shared by all Drawables in this Pass
	//	\date	2022/02/19 19:17
	void addPassBindable( std::shared_ptr<IBindable> bindable ) noexcept;
	//===================================================
	//	\function	bindPass
	//	\brief  bind rtv, dsv & other bindables shared by all Pass objects
	//	\date	2021/06/27 0:04
	void bindPass( Graphics &gph ) const cond_noex;
	void validate() override;

	template<class T>
	void addContainerBindableConsumer( const std::string &name )
	{
		const auto index = m_bindables.size();
		m_bindables.emplace_back();
		addConsumer( std::make_unique<ContainerBindableConsumer<T>>( name,
			m_bindables,
			index ) );
	}
};


}//ren