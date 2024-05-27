#include "imgui_visitors.h"
#include <string>
#include <DirectXMath.h>
#include "imgui/imgui.h"
#include "model.h"
#include "node.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#include "dynamic_constant_buffer.h"
#include "material.h"
#include "utils.h"


namespace dx = DirectX;

bool IImguiNodeVisitor::visit( Node &node )
{
	// if there is no selected node, set selectedNodeId to an impossible value
	const int selectedNodeId = ( m_pSelectedNode == nullptr ) ?
		-1 :
		m_pSelectedNode->getImguiId();

	const int nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ( ( node.getImguiId() == selectedNodeId ) ? ImGuiTreeNodeFlags_Selected : 0 )
		| ( node.hasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf );

	// render this node's imgui
	const bool bNodeOpen = ImGui::TreeNodeEx( (void*)(intptr_t)node.getImguiId(), nodeFlags, node.getName().c_str() );

	if ( ImGui::IsItemClicked() )
	{
		m_pSelectedNode = &node;
	}

	// if bNodeOpen is true then that's the go-signal for children Nodes to also be recursed
	return bNodeOpen;
}

ImguiNodeVisitorShowcase::ImguiNodeVisitorShowcase( const std::string &name )
	:
	m_name{name}
{

}

void ImguiNodeVisitorShowcase::onVisit( Node &node )
{
	ImGui::TreePop();
}

void ImguiNodeVisitorShowcase::displayImguiWidgets( Model &model ) noexcept
{
	ImGui::Begin( m_name.c_str() );
	ImGui::Columns( 2, nullptr, true );
	model.accept( *this );

	ImGui::NextColumn();
	if ( m_pSelectedNode != nullptr )
	{
		bool bDirty = false;
		const auto dirtyCheck = [&bDirty]( const bool bChanged )
		{
			bDirty = bDirty || bChanged;
		};

		// for transforming the model
		auto &tf = calcTransform();

		ImGui::TextColored( { 0.4f, 1.0f, 0.6f, 1.0f }, "Rotation" );
		dirtyCheck( ImGui::SliderAngle( "X-rotation", &tf.pitch, 0.995f * -90.0f, 0.995f * 90.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Y-rotation", &tf.yaw, -180.0f, 180.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Z-rotation", &tf.roll, -180.0f, 180.0f ) );

		ImGui::TextColored( { 0.4f, 1.0f, 0.6f, 1.0f }, "Translation" );
		dirtyCheck( ImGui::SliderFloat( "X", &tf.x, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Y", &tf.y, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Z", &tf.z, -60.f, 60.f ) );

		if ( bDirty )
		{
			m_pSelectedNode->setWorldTransform( dx::XMMatrixRotationX( tf.pitch ) * dx::XMMatrixRotationY( tf.yaw ) * dx::XMMatrixRotationZ( tf.roll ) * dx::XMMatrixTranslation( tf.x, tf.y, tf.z ) );
		}

		ImguiMaterialVisitorShowcase ev;
		m_pSelectedNode->accept( ev );
	}
	ImGui::End();
}

ImguiNodeVisitorShowcase::TransformData& ImguiNodeVisitorShowcase::calcTransform() noexcept
{
	const auto imguiNodeId = m_pSelectedNode->getImguiId();
	auto it = m_nodeMapTransforms.find( imguiNodeId );
	if ( it == m_nodeMapTransforms.end() )
	{
		const auto &mat = m_pSelectedNode->getWorldTransformAccess();
		const auto angles = util::extractRotation( mat );
		const auto translation = util::extractTranslation( mat );

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

////////////////////////////////////////////////////////////////////////////////////////////////////
void IImGuiMaterialVisitor::setMaterial( Material *pMaterial )
{
	m_pMaterial = pMaterial;
	++m_materialId;
	onSetMaterial();
}

bool IImGuiMaterialVisitor::visit( con::CBuffer &cb )
{
	++m_imguiId;
	return onVisit( cb );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ImguiMaterialVisitorShowcase::onVisit( con::CBuffer &cb )
{
	bool bDirty = false;
	const auto dirtyCheck = [&bDirty]( const bool bChanged )
	{
		bDirty = bDirty || bChanged;
	};

	auto tagImGuiWidget = [imguiNodeLabel = std::string{},
		strImguiId = '#' + std::to_string( m_imguiId )]
		( const char *label ) mutable
	{
		imguiNodeLabel = label + strImguiId;
		return imguiNodeLabel.c_str();
	};

	// query the Mesh's Constant Buffer for available CB Elements and display ImGui controls for those available
	if ( const auto &el = cb["scale"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Scale" ), &el, 1.0f, 2.0f, "%.3f", 3.5f ) );
	}

	if ( const auto &el = cb["offset"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "offset" ), &el, 0.0f, 1.0f, "%.3f", 2.5f ) );
	}

	if ( const auto &el = cb["materialColor"]; el.isValid() )
	{
		dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "Diff. Color" ), reinterpret_cast<float*>( &static_cast<dx::XMFLOAT3&>( el ) ) ) );
	}

	if ( const auto &el = cb["modelSpecularColor"]; el.isValid() )
	{
		dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "Spec. Color" ), reinterpret_cast<float*>( &static_cast<dx::XMFLOAT3&>( el ) ) ) );
	}

	if ( const auto &el = cb["modelSpecularGloss"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Gloss" ), &el, 1.0f, 100.0f, "%.1f", 1.5f ) );
	}

	if ( const auto &el = cb["useSpecularMap"]; el.isValid() )
	{
		dirtyCheck( ImGui::Checkbox( tagImGuiWidget( "Enable Specular Map" ), &el ) );
	}

	if ( const auto &el = cb["useNormalMap"]; el.isValid() )
	{
		dirtyCheck( ImGui::Checkbox( tagImGuiWidget( "Enable Normal Map" ), &el ) );
	}

	if ( const auto &el = cb["normalMapWeight"]; el.isValid() )
	{
		dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Normal Map Weight" ), &el, 0.0f, 2.0f ) );
	}

	return bDirty;
}

void ImguiMaterialVisitorShowcase::onSetMaterial()
{
	ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, util::capitalizeFirstLetter( m_pMaterial->getTargetPassName() ).c_str() );

	bool bActive = m_pMaterial->isEnabled();
	using namespace std::string_literals;
	ImGui::Checkbox( ( "Material Active#"s + std::to_string( m_materialId ) ).c_str(), &bActive );
	m_pMaterial->setEnabled( bActive );
}