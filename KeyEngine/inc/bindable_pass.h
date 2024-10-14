#pragma once

#include <memory>
#include "pass.h"
#include "binder.h"


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
	std::shared_ptr<IRenderTargetView> m_pRtv;
	std::shared_ptr<IDepthStencilView> m_pDsv;
protected:
	IBindablePass( const std::string &name, const std::vector<std::shared_ptr<IBindable>> &bindables = {} );

	//	\function	addBindable	||	\date	2022/02/19 19:17
	//	\brief	add Bindables that will be shared by all meshes in this Pass
	void addBindable( std::shared_ptr<IBindable> bindable ) noexcept;
	//	\function	bind	||	\date	2021/06/27 0:04
	//	\brief	bind RTV or DSV and other bindables shared by all Pass objects
	//			call this function as the first thing you do on a child class's run function
	void bind( Graphics &gfx ) const cond_noex;
	void validate() override;

	template<class T>
	void addBindableBinderUsingContainerIndex( const std::string &binderName )
	{
		m_bindables.emplace_back();
		const auto index = m_bindables.size() - 1;
		addBinder( std::make_unique<ContainerBindableBinder<T>>( binderName, m_bindables, index ) );
	}
};


}//ren