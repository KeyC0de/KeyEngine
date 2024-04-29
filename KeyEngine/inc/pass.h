#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>


class Graphics;

namespace ren
{

class IBinder;
class ILinker;

//=============================================================
//	\class	IPass
//	\author	KeyC0de
//	\date	2021/10/20 16:28
//	\brief	a rendering pass for all models with same material - GPU bindings
//=============================================================
class IPass
{
	std::string m_name;
	bool m_bActive;
	std::vector<std::unique_ptr<IBinder>> m_binders;
	std::vector<std::unique_ptr<ILinker>> m_linkers;
public:
	IPass( const std::string &name, bool bActive = true ) noexcept;
	virtual ~IPass() noexcept;

	//	\function	run	||	\date	2021/06/27 0:51
	//	\brief  binds bindables & executes draw calls
	virtual void run( Graphics &gph ) const cond_noex = 0;
	virtual void reset() cond_noex = 0;
	const std::string& getName() const noexcept;
	const std::vector<std::unique_ptr<IBinder>>& getBinders() const;
	const std::vector<std::unique_ptr<ILinker>>& getLinkers() const;
	IBinder& getBinder( const std::string &name ) const;
	ILinker& getLinker( const std::string &name ) const;
	//	\function	setupBinderTarget	||	\date	2021/06/28 0:30
	//	\brief  link a Binder from this Pass to a Linker of another targetPass
	void setupBinderTarget( const std::string &binderName, const std::string &targetPassName, const std::string &targetPassLinkerName );
	//	\function	validate	||	\date	2022/02/19 22:48
	//	\brief  validate binders are linked to their linkers
	//			validation occurs only once (ctor) for every Pass
	virtual void validate();
	void setActive( const bool bActive ) noexcept;
	const bool isActive() const noexcept;
protected:
	void addBinder( std::unique_ptr<IBinder> pBinder );
	void addLinker( std::unique_ptr<ILinker> pLinker );
};


}//ren