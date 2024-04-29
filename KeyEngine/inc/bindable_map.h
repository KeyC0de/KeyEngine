#pragma once

#include <type_traits>
#include <memory>
#include <unordered_map>
#include "bindable.h"


// #TODO: replace shared_ptr to IBindable with weak_ptr to get rid of the usefulness of garbage collection
class BindableMap final
{
	std::unordered_map<std::string, std::shared_ptr<IBindable>> m_bindableMap;
public:
	template<class T, typename... TArgs>
	static std::shared_ptr<T> fetch( Graphics &gph,
		TArgs&&... args ) cond_noex
	{
		static_assert( std::is_base_of<IBindable, T>::value, "T must be a IBindable!" );
		return getInstance().fetch_impl<T>( gph, std::forward<TArgs>( args )... );
	}

#if defined _DEBUG && !defined NDEBUG
	static std::size_t getInstanceCount()
	{
		const auto &instance = getInstance();
		return instance.m_bindableMap.size();
	}

	static std::size_t getGarbageCount()
	{
		const auto &instance = getInstance();
		size_t nGarbagePtrs = 0;
		for ( const auto &elem : instance.m_bindableMap )
		{
			if ( elem.second.use_count() <= 1 )
			{
				++nGarbagePtrs;
			}
		}
		return nGarbagePtrs;
	}
#endif

	static void garbageCollect()
	{
		auto &instance = BindableMap::getInstance();
		for ( std::unordered_map<std::string, std::shared_ptr<IBindable>>::iterator it = instance.m_bindableMap.begin(); it != instance.m_bindableMap.end(); ++it )
		{
			if ( it->second.use_count() <= 1 )
			{
				instance.m_bindableMap.erase( it );
				it = instance.m_bindableMap.begin();
			}
		}
	}
private:
	template<class T, typename... TArgs>
	std::shared_ptr<T> fetch_impl( Graphics &gph,
		TArgs&&... args ) cond_noex
	{
		const std::string bindableId = T::calcUid( std::forward<TArgs>( args )... );
		const auto /*std::unordered_map<std::string, std::shared_ptr<IBindable>>::const_iterator*/ elem = m_bindableMap.find( bindableId );
		if ( elem == m_bindableMap.cend() )
		{
			std::shared_ptr<T> bindable = std::make_shared<T>( gph, std::forward<TArgs>( args )... );
			m_bindableMap[bindableId] = bindable;
			return bindable;
		}
		else
		{
			return std::dynamic_pointer_cast<T>( elem->second );
		}
	}

	static BindableMap& getInstance()
	{
		static BindableMap instance;
		return instance;
	}
};