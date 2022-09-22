#include "model_visitor.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "model.h"
#include "node.h"
#include "effect_visitor.h"
#include "d3d_utils.h"
#include "assertions_console.h"


namespace dx = DirectX;


ImguiVisitor::ImguiVisitor( const std::string &name )
	:
	m_name{name}
{

}

//ImguiVisitor::~ImguiVisitor() noexcept
//{
//	m_pSelectedNode = nullptr;
//}

void ImguiVisitor::spawnModelImgui( Model &model )
{
	ImGui::Begin( m_name.c_str() );
	ImGui::Columns( 2,
		nullptr,
		true );
	model.accept( *this );

	ImGui::NextColumn();
	if ( m_pSelectedNode != nullptr )
	{
		bool bDirty = false;
		const auto dirtyCheck = [&bDirty]( bool bChanged )
		{
			bDirty = bDirty || bChanged;
		};

		// for transforming the model
		auto &tf = calcTransform();

		ImGui::TextColored( { 0.4f, 1.0f, 0.6f, 1.0f }, "Translation" );
		dirtyCheck( ImGui::SliderFloat( "X", &tf.x, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Y", &tf.y, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Z", &tf.z, -60.f, 60.f ) );

		ImGui::TextColored( { 0.4f, 1.0f, 0.6f, 1.0f }, "Rotation" );
		dirtyCheck( ImGui::SliderAngle( "X-rotation", &tf.pitch, -180.0f, 180.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Y-rotation", &tf.yaw, -180.0f, 180.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Z-rotation", &tf.roll, -180.0f, 180.0f ) );

		if ( bDirty )
		{
			m_pSelectedNode->setWorldTransform( dx::XMMatrixRotationX( tf.pitch ) *
				dx::XMMatrixRotationY( tf.yaw ) *
				dx::XMMatrixRotationZ( tf.roll ) *
				dx::XMMatrixTranslation( tf.x, tf.y, tf.z ) );
		}

		EVShowcase ev;
		m_pSelectedNode->accept( ev );
	}
	ImGui::End();
}

const std::string& ImguiVisitor::getName() const noexcept
{
	return m_name;
}

bool ImguiVisitor::visit( Node &node )
{
	// if there is no selected node, set selectedNodeId to an impossible value
	const int selectedNodeId = ( m_pSelectedNode == nullptr ) ? -1 :
		m_pSelectedNode->getImguiId();
	
	// build up flags for current node
	const auto nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ( ( node.getImguiId() == selectedNodeId ) ? ImGuiTreeNodeFlags_Selected : 0 )
		| ( node.hasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf );

	// render this node
	const bool bNodeOpen = ImGui::TreeNodeEx( (void*)(intptr_t)node.getImguiId(),
		nodeFlags,
		node.getName().c_str() );
	
	if ( ImGui::IsItemClicked() )
	{
		m_pSelectedNode = &node;
	}
	
	// if bNodeOpen is true then that's the go-signal for children Nodes to also be recursed
	return bNodeOpen;
}

void ImguiVisitor::onVisited( Node &node )
{
	ImGui::TreePop();
}

ImguiVisitor::TransformData& ImguiVisitor::calcTransform() noexcept
{
	const auto imguiNodeId = m_pSelectedNode->getImguiId();
	auto it = m_nodeMapTransforms.find( imguiNodeId );
	if ( it == m_nodeMapTransforms.end() )
	{
		const auto &worldTf = m_pSelectedNode->getWorldTransform();
		const auto angles = util::extractEulerAngles( worldTf );
		const auto translation = util::extractTranslation( worldTf );

		TransformData td;
		td.pitch = angles.x;
		td.yaw = angles.y;
		td.roll = angles.z;
		td.x = translation.x;
		td.y = translation.y;
		td.z = translation.z;
	
		bool bInserted = false;
		std::tie( it, bInserted ) = m_nodeMapTransforms.insert( {imguiNodeId, {td}} );
		ASSERT( bInserted, "Transform not inserted or value already in the unordered_map!" );
	}
	return it->second;
}