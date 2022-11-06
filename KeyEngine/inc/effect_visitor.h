#pragma once

#include <limits>


namespace con
{
	class CBuffer;
}

class Effect;

//============================================================
//	\class	IEffectVisitor
//	\author	KeyC0de
//	\date	2020/01/09 15:53
//	\brief	implement and override on* methods
//			targets a Model with a specific effect
//			the IDs are for tagging ImGui widgets/controls
//=============================================================
class IEffectVisitor
{
protected:
	Effect *m_pEffect = nullptr;
	size_t m_effectId = std::numeric_limits<size_t>::max();
	size_t m_imguiId = std::numeric_limits<size_t>::max();
public:
	virtual ~IEffectVisitor();

	void setEffect( Effect *ef );
	//	\function	visit	||	\date	2022/08/31 11:33
	//	\brief	returns true if concrete bindable (with a con::CBuffer) requires an update
	bool visit( con::CBuffer &cb );
protected:
	virtual void onSetEffect();
	virtual bool onVisit( con::CBuffer &cb ) = 0;
};

#ifndef FINAL_RELEASE
class EVShowcase
	: public IEffectVisitor
{
public:
	bool onVisit( con::CBuffer &cb ) override;
	void onSetEffect() override;
};
#endif