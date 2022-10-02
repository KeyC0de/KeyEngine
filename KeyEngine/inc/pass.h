#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>


class Graphics;

namespace ren
{

class IConsumer;
class IProducer;

//=============================================================
//	\class	IPass
//
//	\author	KeyC0de
//	\date	2021/10/20 16:28
//
//	\brief	a rendering pass for all models with same material - GPU bindings
//			a Consumer consumes what a Producer produces
//=============================================================
class IPass
{
	std::string m_name;
	bool m_bActive;
	std::vector<std::unique_ptr<IConsumer>> m_consumers;
	std::vector<std::unique_ptr<IProducer>> m_producers;
public:
	IPass( const std::string &name, bool bActive = true ) noexcept;
	virtual ~IPass() noexcept;

	//===================================================
	//	\function	run
	//	\brief  binds bindables & executes draw calls
	//	\date	2021/06/27 0:51
	virtual void run( Graphics &gph ) const cond_noex = 0;
	virtual void reset() cond_noex = 0;
	const std::string& getName() const noexcept;
	const std::vector<std::unique_ptr<IConsumer>>& getConsumers() const;
	const std::vector<std::unique_ptr<IProducer>>& getProducers() const;
	IConsumer& consumer( const std::string &name ) const;
	IProducer& producer( const std::string &name ) const;
	//===================================================
	//	\function	setupConsumerTarget
	//	\brief  link a consumer from this Pass to a producer of another targetPass
	//	\date	2021/06/28 0:30
	void setupConsumerTarget( const std::string &consumerName, const std::string &targetPassName, const std::string &targetPassProducerName );
	//===================================================
	//	\function	validate
	//	\brief  validate consumers are linked to their producers
	//			validation occurs only once (ctor) for every Pass
	//	\date	2022/02/19 22:48
	virtual void validate();
	void setActive( const bool bActive ) noexcept;
	const bool isActive() const noexcept;
protected:
	void addConsumer( std::unique_ptr<IConsumer> pConsumer );
	void addProducer( std::unique_ptr<IProducer> pProducer );
};


}//ren