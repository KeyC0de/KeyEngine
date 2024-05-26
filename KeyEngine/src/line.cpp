#include "line.h"
#include "geometry.h"
#include "dynamic_constant_buffer.h"
#include "index_buffer.h"
#include "vertex_buffer.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "input_layout.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#include "blend_state.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "rendering_channel.h"
#include "utils.h"


namespace dx = DirectX;

Line::Line( Graphics &gfx,
	const float length /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT4 &color /*= {1.0f, 0.4f, 0.4f, 1.0f}*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos},
	m_colorPscb{color}
{
	auto line = Geometry::makeLine();
	if ( length != 1.0f )
	{
		line.transform( dx::XMMatrixScaling( length, length, length ) );
	}
	if ( initialRot.x != 0.0f && initialRot.y != 0.0f && initialRot.z != 0.0f )
	{
		line.transform( dx::XMMatrixRotationRollPitchYaw( initialRot.x, initialRot.y, initialRot.z ) );
	}

	//line.setFlatShadedIndependentNormals();
	const auto geometryTag = s_geometryTag + std::to_string( length );

	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, line.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, line.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	using Type = ver::VertexInputLayout::ILEementType;
	m_aabb = std::make_pair( line.m_vb[0].getElement<Type::Position3D>(), line.m_vb[1].getElement<Type::Position3D>() );
	setMeshId();

	auto transformVscb = std::make_shared<TransformVSCB>( gfx, 0u );
	if ( color.w < 1.0f )
	{// transparent reflectance effect
		Effect transparent{rch::transparent, "transparent", true};
		transparent.addBindable( transformVscb );

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		transparent.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, color.w ) );

		addEffect( std::move( transparent ) );
	}
	else
	{// opaque reflectance effect

		Effect opaque{rch::opaque, "opaque", true};
		opaque.addBindable( transformVscb );

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Both ) );

		addEffect( std::move( opaque ) );
	}
	{// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", false};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gfx, line.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
}

void Line::displayImguiWidgets( Graphics &gfx,
	const std::string &name ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( name.c_str() ) )
	{
		bool bDirtyRot = false;
		const auto dirtyCheckRot = []( const bool bChanged, bool &bDirtyRot )
		{
			bDirtyRot = bDirtyRot || bChanged;
		};

		ImGui::Text( "Orientation" );
		dx::XMFLOAT3 rot = getRotation();
		dirtyCheckRot( ImGui::SliderAngle( "Pitch", &rot.x, 0.995f * -90.0f, 0.995f * 90.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Yaw", &rot.y, -180.0f, 180.0f ), bDirtyRot );
		dirtyCheckRot( ImGui::SliderAngle( "Roll", &rot.z, -180.0f, 180.0f ), bDirtyRot );
		if ( bDirtyRot )
		{
			setRotation( rot );
		}

		bool bDirtyPos = false;
		const auto dirtyCheckPos = []( const bool bChanged, bool &bDirtyPos )
		{
			bDirtyPos = bDirtyPos || bChanged;
		};

		ImGui::Text( "Position" );
		dx::XMFLOAT3 pos = getPosition();
		dirtyCheckPos( ImGui::SliderFloat( "X", &pos.x, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Y", &pos.y, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		dirtyCheckPos( ImGui::SliderFloat( "Z", &pos.z, -80.0f, 80.0f, "%.1f" ), bDirtyPos );
		if ( bDirtyPos )
		{
			setPosition( pos );
		}

		auto pBlendState = findBindable<BlendState>();
		if ( pBlendState )
		{
			bool bDirtyAlpha = false;
			const auto dirtyCheckAlpha = []( const bool bChanged, bool &bDirtyAlpha )
			{
				bDirtyAlpha = bDirtyAlpha || bChanged;
			};

			ImGui::Text( "Blending" );
			float factor = (*pBlendState)->getBlendFactorAlpha();
			dirtyCheckAlpha( ImGui::SliderFloat( "Transparency", &factor, 0.0f, 1.0f ), bDirtyAlpha);

			if ( bDirtyAlpha )
			{
				(*pBlendState)->fillBlendFactors( factor );
			}
		}

		class EVLine
			: public IImGuiEffectVisitor
		{
		public:
			void onSetEffect() override
			{
				ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f}, util::capitalizeFirstLetter( m_pEffect->getTargetPassName() ).c_str() );

				bool active = m_pEffect->isEnabled();
				using namespace std::string_literals;
				ImGui::Checkbox( ( "Effect Active#"s + std::to_string( m_effectId ) ).c_str(), &active );
				m_pEffect->setEnabled( active );
			}

			bool onVisit( con::CBuffer &cb ) override
			{
				bool bDirty = false;
				const auto dirtyCheck = [&bDirty]( const bool bChanged )
				{
					bDirty = bDirty || bChanged;
				};
				auto tagImGuiWidget = [imguiNodeLabel = std::string{},
					strImguiId = "#" + std::to_string( m_imguiId )]
					( const char *label ) mutable
					{
						imguiNodeLabel = label + strImguiId;
						return imguiNodeLabel.c_str();
					};

				if ( auto el = cb["scale"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Scale" ), &el, 1.0f, 2.0f, "%.3f", 3.5f ) );
				}

				if ( auto el = cb["materialColor"]; el.isValid() )
				{
					dirtyCheck( ImGui::ColorPicker4( tagImGuiWidget( "materialColor" ), reinterpret_cast<float*>( &static_cast<dx::XMFLOAT4&>( el ) ) ) );
				}

				if ( auto el = cb["modelSpecularGloss"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Glossiness" ), &el, 1.0f, 200.0f, "%.1f", 1.5f ) );
				}

				return bDirty;
			}
		} evLine;

		accept( evLine );
	}
	ImGui::End();
#endif
}