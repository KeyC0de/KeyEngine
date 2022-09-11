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
	IBindablePass( const std::string &name, const std::vector<std::shared_ptr<IBindable>>& bindables = {} );

	//===================================================
	//	\function	addPassBindable
	//	\brief  add Bindables that will be shared by all meshes in this Pass
	//	\date	2022/02/19 19:17
	void addPassBindable( std::shared_ptr<IBindable> bindable ) noexcept;
	//===================================================
	//	\function	bind
	//	\brief  bind RTV OR DSV and other bindables shared by all Pass objects
	//			call this function as the first thing you do on a child class's run function
	//	\date	2021/06/27 0:04
	void bind( Graphics &gph ) const cond_noex;
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