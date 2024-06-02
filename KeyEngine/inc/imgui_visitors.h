#pragma once

#include <string>
#include <unordered_map>
#include <limits>


class Model;
class Node;
class Material;

namespace con
{
	class CBuffer;
}

class IImguiNodeVisitor
{
protected:
	Node *m_pSelectedNode = nullptr;
public:
	virtual ~IImguiNodeVisitor() noexcept = default;

	//	\function	visit	||	\date	2022/09/20 23:06
	//	\brief	returns true if we want to "open" the Node in the ImGui tree hierarchy for processing of its children
	bool visit( Node &node );
	virtual void onVisit( class Node &node ) = 0;
};

class ImguiPerModelNodeVisitor final
	: IImguiNodeVisitor
{
	std::string m_name;
	struct TransformData
	{
		float pitch = 0.0f;
		float yaw = 0.0f;
		float roll = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	std::unordered_map<int, TransformData> m_nodeMapTransforms;
public:
	ImguiPerModelNodeVisitor() = default;
	ImguiPerModelNodeVisitor( const std::string &name );

	//	\function	onVisit	||	\date	2022/09/20 23:07
	//	\brief	if visit() returns true, "close" the node
	void onVisit( Node &node ) override;
	void displayImguiWidgets( Model &model ) noexcept;
private:
	TransformData& getNodeTransform() noexcept;
};

#ifdef max
#	undef max
#endif

//============================================================
//	\class	IImGuiConstantBufferVisitor
//	\author	KeyC0de
//	\date	2020/01/09 15:53
//	\brief	Visitor that operates on a material that contains a con::CBuffer and its Bindable(s) override Bindable::accept() which ultimately call IImGuiConstantBufferVisitor::visit( cb )
//			targets a Model with a specific material
//			the IDs are for tagging ImGui widgets/controls
//=============================================================
class IImGuiConstantBufferVisitor
{
protected:
	Material *m_pMaterial = nullptr;
	size_t m_materialId = std::numeric_limits<size_t>::max();
	size_t m_imguiId = std::numeric_limits<size_t>::max();
public:
	virtual ~IImGuiConstantBufferVisitor() noexcept = default;

	void setMaterial( Material *ef );
	//	\function	visit	||	\date	2022/08/31 11:33
	//	\brief	returns true if concrete bindable (with a con::CBuffer) requires an update
	bool visit( con::CBuffer &cb );
	virtual bool onVisit( con::CBuffer &cb ) = 0;
	virtual void onSetMaterial() = 0;
};

class ImguiConstantBufferVisitorShowcase final
	: public IImGuiConstantBufferVisitor
{
public:
	bool onVisit( con::CBuffer &cb ) override;
	void onSetMaterial() override;
};
