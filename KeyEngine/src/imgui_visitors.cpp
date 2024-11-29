#include "imgui_visitors.h"
#ifndef FINAL_RELEASE
#include <DirectXMath.h>
#include <variant>
#include "model.h"
#include "mesh.h"
#include "node.h"
#include "blend_state.h"
#include "d3d_utils.h"
#include "assertions_console.h"
#include "dynamic_constant_buffer.h"
#include "material.h"
#include "utils.h"
#include "math_utils.h"
#include "imgui/imgui.h"


namespace dx = DirectX;

bool IImguiNodeVisitor::visit( Node &node )
{
	// if there is no selected node, set selectedNodeId to an impossible value
	const int selectedNodeId = ( m_pSelectedNode == nullptr ) ? -1 : m_pSelectedNode->getImguiId();

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

ImguiPerModelNodeVisitor::ImguiPerModelNodeVisitor( const std::string &name )
	:
	m_name{name}
{

}

void ImguiPerModelNodeVisitor::onVisit( Node &node )
{
	ImGui::TreePop();
}

void ImguiPerModelNodeVisitor::displayImguiWidgets( Model &model ) noexcept
{
	ImGui::Begin( m_name.c_str() );
	ImGui::Columns( 2, nullptr, true );
	model.accept( *this );

	ImGui::NextColumn();


	if ( m_pSelectedNode != nullptr )
	{

		bool bDirtyRot = false;
		const auto dirtyCheckRot = []( const bool bChanged, bool &bDirtyRot )
		{
			bDirtyRot = bDirtyRot || bChanged;
		};

		// for transforming the Node
		auto &tf = getNodeTransform();

		// #WARNING: ImGui::SliderAngle & similar Rotation functions take their arguments in radians convert it in degrees and return it back in radians
		ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, "Rotation" );
		dirtyCheckRot( ImGui::SliderAngle( "Pitch", &tf.pitch, -0.995f * 90.0f, 0.995f * 90.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Yaw", &tf.yaw, -180.0f, 180.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Roll", &tf.roll, -180.0f, 180.0f ), bDirtyRot );
		if ( bDirtyRot )
		{
			m_pSelectedNode->setRotation( {tf.pitch, tf.yaw, tf.roll} );
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = []( const bool bChanged, bool &bDirtyPos )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};

		ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, "Translation" );
		dirtyCheckPos( ImGui::SliderFloat( "X", &tf.x, -60.f, 60.f ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &tf.y, -60.f, 60.f ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &tf.z, -60.f, 60.f ), bDirtyPos );
		if ( bDirtyPos )
		{
			m_pSelectedNode->setTranslation( {tf.x, tf.y, tf.z} );
		}

		/*
		bool bDirty = false;
		const auto dirtyCheck = [&bDirty]( const bool bChanged )
		{
			bDirty = bDirty || bChanged;
		};

		// for transforming the model
		auto &tf = getNodeTransform();

		ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, "Rotation" );
		dirtyCheck( ImGui::SliderAngle( "Pitch", &tf.pitch, 0.995f * -90.0f, 0.995f * 90.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Yaw", &tf.yaw, -180.0f, 180.0f ) );
		dirtyCheck( ImGui::SliderAngle( "Roll", &tf.roll, -180.0f, 180.0f ) );

		ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, "Translation" );
		dirtyCheck( ImGui::SliderFloat( "X", &tf.x, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Y", &tf.y, -60.f, 60.f ) );
		dirtyCheck( ImGui::SliderFloat( "Z", &tf.z, -60.f, 60.f ) );

		if ( bDirty )
		{
			m_pSelectedNode->setTransform( dx::XMMatrixScaling( 1.0f, 1.0f, 1.0f ) * dx::XMMatrixRotationX( tf.pitch ) * dx::XMMatrixRotationY( tf.yaw ) * dx::XMMatrixRotationZ( tf.roll ) * dx::XMMatrixTranslation( tf.x, tf.y, tf.z ) );
		}
		*/

		auto pBlendState = model.getMesh() ? model.getMesh()->findBindable<BlendState>() : std::nullopt;
		if ( pBlendState )
		{
			bool bDirtyBlend = false;
			const auto dirtyCheckBlend = []( const bool bChanged, bool &bDirtyBlend )
			{
				bDirtyBlend = bDirtyBlend || bChanged;
			};

			ImGui::Text( "Blending" );
			float factor = (*pBlendState)->getBlendFactorAlpha();
			dirtyCheckBlend( ImGui::SliderFloat( "Transparency", &factor, 0.0f, 1.0f ), bDirtyBlend);

			if ( bDirtyBlend )
			{
				(*pBlendState)->fillBlendFactors( factor );
			}
		}

		ImguiConstantBufferVisitorShowcase ev;
		m_pSelectedNode->accept( ev );
	}
	ImGui::End();
}

ImguiPerModelNodeVisitor::TransformData& ImguiPerModelNodeVisitor::getNodeTransform() noexcept
{
	const auto imguiNodeId = m_pSelectedNode->getImguiId();
	auto it = m_nodeMapTransforms.find( imguiNodeId );
	if ( it == m_nodeMapTransforms.end() )
	{
		const auto angles = m_pSelectedNode->getRotation();
		const auto translation = m_pSelectedNode->getPosition();

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
void IImGuiConstantBufferVisitor::setMaterial( Material *pMaterial )
{
	m_pMaterial = pMaterial;
	++m_materialId;
	onSetMaterial();
}

bool IImGuiConstantBufferVisitor::visit( con::CBuffer &cb )
{
	++m_imguiId;
	return onVisit( cb );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ImguiConstantBufferVisitorShowcase::onVisit( con::CBuffer &cb )
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
		dirtyCheck( ImGui::ColorPicker4( tagImGuiWidget( "Diff. Color" ), reinterpret_cast<float*>( &static_cast<dx::XMFLOAT4&>( el ) ) ) );
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

void ImguiConstantBufferVisitorShowcase::onSetMaterial()
{
	ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, util::capitalizeFirstLetter( m_pMaterial->getTargetPassName() ).c_str() );

	bool bActive = m_pMaterial->isEnabled();
	using namespace std::string_literals;
	ImGui::Checkbox( ( "Material Active#"s + std::to_string( m_materialId ) ).c_str(), &bActive );
	m_pMaterial->setEnabled( bActive );
}
#endif