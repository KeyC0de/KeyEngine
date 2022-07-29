#pragma once

#include <type_traits>
#include <memory>
#include <unordered_map>
#include "bindable.h"


class BindableMap final
{
	std::unordered_map<std::string, std::shared_ptr<IBindable>> m_bindableMap;
public:
	template<class T, typename... TArgs>
	static std::shared_ptr<T> fetch( Graphics &gph,
		TArgs&&... args ) cond_noex
	{
		static_assert( std::is_base_of<IBindable, T>::value, "T must be a IBindable!" );
		return getInstance().fetch_impl<T>( gph,
				std::forward<TArgs>( args )... );
	}
private:
	template<class T, typename... TArgs>
	std::shared_ptr<T> fetch_impl( Graphics &gph,
		TArgs&&... args ) cond_noex
	{
		const auto bindableId = T::generateUid( std::forward<TArgs>( args )... );
		const auto elem = m_bindableMap.find( bindableId );
		if ( elem == m_bindableMap.end() )
		{
			auto bindable = std::make_shared<T>( gph,
				std::forward<TArgs>( args )... );
			m_bindableMap[bindableId] = bindable;
			return bindable;
		}
		else
		{
			return std::dynamic_pointer_cast<T>( elem->second );
		}
	}

	static BindableMap &getInstance()
	{
		static BindableMap instance;
		return instance;
	}
};