#pragma once

#include <limits>


namespace con
{
	class Buffer;
}

class Effect;

//============================================================
//	\class	IEffectVisitor
//
//	\author	KeyC0de
//	\date	2020/01/09 15:53
//
//	\brief	implement and override on *methods
//			targets a model with a specific effect
//			the IDs are for tagging ImGui widgets/controls
//=============================================================
class IEffectVisitor
{
protected:
	Effect *m_pEffect = nullptr;
	size_t m_effectId = std::numeric_limits<size_t>::max();
	size_t m_cbId = std::numeric_limits<size_t>::max();
public:
	virtual ~IEffectVisitor();

	void setEffect( Effect *ef );
	// if concrete bindable (with a con::Buffer) requires an update onVisit returns true
	bool visit( con::Buffer &cb );
protected:
	virtual void onSetEffect();
	virtual bool onVisit( con::Buffer &cb ) = 0;
};

class EVShowcase
	: public IEffectVisitor
{
public:
	bool onVisit( con::Buffer &cb ) override;
	void onSetEffect() override;
};