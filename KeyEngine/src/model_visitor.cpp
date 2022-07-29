#include "model_visitor.h"
#include <DirectXMath.h>
#include "imgui.h"
#include "model.h"
#include "node.h"
#include "effect_visitor.h"
#include "d3d_utils.h"


namespace dx = DirectX;


MV::MV( const std::string &name )
	:
	m_name{name}
{

}

void MV::spawnModelImgui( Model &model )
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

		// for transforming the .obj model whole
		auto &tf = fetchTransform();

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
			m_pSelectedNode->setTransform( dx::XMMatrixRotationX( tf.pitch ) *
				dx::XMMatrixRotationY( tf.yaw ) *
				dx::XMMatrixRotationZ( tf.roll ) *
				dx::XMMatrixTranslation( tf.x, tf.y, tf.z ) );
		}

		EVShowcase ev;
		m_pSelectedNode->accept( ev );
	}
	ImGui::End();
}

bool MV::visit( Node &node )
{
	// if there is no selected node, set selectedNodeId to an impossible value
	const int selectedNodeId = ( m_pSelectedNode == nullptr ) ?
		-1 :
		m_pSelectedNode->getId();
	// build up flags for current node
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
		| ( ( node.getId() == selectedNodeId ) ? ImGuiTreeNodeFlags_Selected : 0 )
		| ( node.hasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf );
	// render this node
	const bool bExpand = ImGui::TreeNodeEx( (void*)(intptr_t)node.getId(),
		node_flags,
		node.getName().c_str() );
	// processing for selecting node
	if ( ImGui::IsItemClicked() )
	{
		m_pSelectedNode = &node;
	}
	// signal if children should also be recursed
	return bExpand;
}

void MV::onNodeLeave( Node &node )
{
	ImGui::TreePop();
}

MV::TransformData &MV::fetchTransform() noexcept
{
	const auto nodeId = m_pSelectedNode->getId();
	auto i = m_nodeMapTransforms.find( nodeId );
	if ( i == m_nodeMapTransforms.end() )
	{
		return assembleTransform( nodeId );
	}
	return i->second;
}

MV::TransformData &MV::assembleTransform( int id ) noexcept
{
	const auto &worldTf = m_pSelectedNode->getWorldTransform();
	const auto angles = util::extractEulerAngles( worldTf );
	const auto translation = util::extractTranslation( worldTf );

	TransformData td;
	td.roll = angles.z;
	td.pitch = angles.x;
	td.yaw = angles.y;
	td.x = translation.x;
	td.y = translation.y;
	td.z = translation.z;
	return m_nodeMapTransforms.insert( {id, {td}} ).first->second;
}