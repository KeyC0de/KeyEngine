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
//	\brief	IPass stands for a Model Pass, ie. a pass for all models with same
//				material - GPU bindings. It's not a rendering pass (draw call to a RT).
//=============================================================
class IPass
{
	std::vector<std::unique_ptr<IConsumer>> m_consumers;
	std::vector<std::unique_ptr<IProducer>> m_producers;
	std::string m_name;
public:
	IPass( const std::string &name ) noexcept;
	virtual ~IPass();

	//===================================================
	//	\function	run
	//	\brief  binds bindables & executes draw calls
	//	\date	2021/06/27 0:51
	virtual void run( Graphics &gph ) const cond_noex = 0;
	virtual void reset() cond_noex = 0;
	const std::string &getName() const noexcept;
	const std::vector<std::unique_ptr<IConsumer>>& getConsumers() const;
	const std::vector<std::unique_ptr<IProducer>>& getProducers() const;
	IConsumer &getConsumer( const std::string &name ) const;
	IProducer &getProducer( const std::string &name ) const;
	//===================================================
	//	\function	setupConsumerTarget
	//	\brief  link a consumer from this Pass to a producer of another targetPass
	//	\date	2021/06/28 0:30
	void setupConsumerTarget( const std::string &consumerName, const std::string &targetPassName,
		const std::string &producerName );
	//===================================================
	//	\function	validate
	//	\brief  validate consumers are linked to their producers
	//			validation occurs only once (ctor) for every Pass
	//	\date	2022/02/19 22:48
	virtual void validate();
protected:
	void addConsumer( std::unique_ptr<IConsumer> pConsumer );
	void addProducer( std::unique_ptr<IProducer> pProducer );
};


}//ren