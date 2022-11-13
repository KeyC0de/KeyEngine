#include "terrain.h"
#include "index_buffer.h"
#include "geometry.h"
#include "input_layout.h"
#include "blend_state.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "transform_scale_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "rendering_channel.h"
#include "utils.h"
#include "math_utils.h"
#include "d3d_utils.h"


namespace dx = DirectX;

Terrain::Terrain( Graphics &gph,
	const int length,
	const int width,
	const DirectX::XMFLOAT3 &startingPos /*= {0.0f, 0.0f, 0.0f}*/ )
	:
	m_pos{startingPos}
{
	auto planarGrid = Geometry::makePlanarGridTextured( length,
		width,
		util::ceil( length / 2 )  - 1,
		util::ceil( width / 2 ) - 1 );
	planarGrid.transform( getRotation( {90.0f, 0.0f, 0.0f} ) );

	const auto geometryTag = "terrainGrid." + std::to_string( length ) + std::to_string( width );

	m_pVertexBuffer = VertexBuffer::fetch( gph,
		geometryTag,
		planarGrid.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		geometryTag,
		planarGrid.m_indices );

	createAabb( planarGrid.m_vb );
	setMeshId();

	auto transformVscb = std::make_shared<TransformVSCB>( gph,
		0u );
	{
	// lambertian reflectance effect
		Effect lambertian{rch::lambert, "lambertian", false};
		lambertian.addBindable( transformVscb );

		lambertian.addBindable( Texture::fetch( gph,
			"assets/models/brick_wall/brick_wall_diffuse.jpg",
			0u ) );
		lambertian.addBindable( TextureSamplerState::fetch( gph,
			0u,
			TextureSamplerState::FilterMode::Anisotropic,
			TextureSamplerState::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gph,
			"plane_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph,
			planarGrid.m_vb.getLayout(),
			*pVs ) );
		lambertian.addBindable( std::move( pVs ) );

		lambertian.addBindable( PixelShader::fetch( gph,
			"plane_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
		cb["modelSpecularGloss"] = 20.0f;
		lambertian.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		lambertian.addBindable( RasterizerState::fetch( gph,
			RasterizerState::FrontSided,
			RasterizerState::Solid ) );

		addEffect( std::move( lambertian ) );
	}
	{
	//shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", false};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gph,
			planarGrid.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{
	// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", false};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gph,
			planarGrid.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{
	// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", false};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gph,
			0u,
			1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gph,
			planarGrid.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
	{
		Effect wireframe{rch::wireframe, "wireframe", true};

		auto pVs = VertexShader::fetch( gph,
			"flat_vs.cso" );
		wireframe.addBindable( InputLayout::fetch( gph,
			m_pVertexBuffer->getLayout(),
			*pVs ) );
		wireframe.addBindable( std::move( pVs ) );

		wireframe.addBindable( PixelShader::fetch( gph,
			"flat_ps.cso" ) );

		struct ColorPCB
		{
			dx::XMFLOAT3 color{1.0f, 1.0f, 1.0f};
			float paddingPlaceholder = 0.0f;
		} colorPcb;

		wireframe.addBindable( PixelShaderConstantBuffer<ColorPCB>::fetch( gph,
			colorPcb,
			0u ) );
		wireframe.addBindable( std::make_shared<TransformVSCB>( gph,
			0u ) );

		addEffect( std::move( wireframe ) );
	}
}

void Terrain::setPosition( const dx::XMFLOAT3 &pos ) noexcept
{
	this->m_pos = pos;
}

void Terrain::setRotation( const DirectX::XMFLOAT3 &rot ) noexcept
{
	this->m_rot = rot;
}

const dx::XMMATRIX Terrain::getTransform() const noexcept
{
	return calcRotation() * calcPosition();
}

const dx::XMMATRIX Terrain::calcPosition() const noexcept
{
	return dx::XMMatrixTranslationFromVector( dx::XMLoadFloat3( &m_pos ) );
}

const dx::XMMATRIX Terrain::calcRotation() const noexcept
{
	return dx::XMMatrixRotationRollPitchYawFromVector( dx::XMLoadFloat3( &m_rot ) );
}

void Terrain::displayImguiWidgets( Graphics &gph,
	const char *name ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( name ) )
	{
		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X",
			&m_pos.x,
			-80.0f,
			80.0f,
			"%.1f" );
		ImGui::SliderFloat( "Y",
			&m_pos.y,
			-80.0f,
			80.0f,
			"%.1f" );
		ImGui::SliderFloat( "Z",
			&m_pos.z,
			-80.0f,
			80.0f,
			"%.1f" );
		ImGui::Text( "Orientation" );
		ImGui::SliderAngle( "Pitch",
			&m_rot.x,
			-90.0f,
			90.0f );
		ImGui::SliderAngle( "Yaw",
			&m_rot.y,
			-180.0f,
			180.0f );
		ImGui::SliderAngle( "Roll",
			&m_rot.z,
			-180.0f,
			180.0f );

		class EVTerrain
			: public IEffectVisitor
		{
		public:
			void onSetEffect() override
			{
				ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f},
					util::capitalizeFirstLetter( m_pEffect->getTargetPassName() ).c_str() );

				bool active = m_pEffect->isEnabled();
				using namespace std::string_literals;
				ImGui::Checkbox( ( "Effect Active#"s + std::to_string( m_effectId ) ).c_str(),
					&active );
				m_pEffect->setEnabled( active );
			}

			bool onVisit( con::CBuffer &cb ) override
			{
				bool bDirty = false;
				const auto dirtyCheck = [&bDirty]( bool bChanged )
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
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Scale" ),
						&el,
						1.0f,
						2.0f,
						"%.3f",
						3.5f ) );
				}

				if ( auto el = cb["materialColor"]; el.isValid() )
				{
					dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "materialColor" ),
						reinterpret_cast<float*>( &static_cast<dx::XMFLOAT3&>( el ) ) ) );
				}

				if ( auto el = cb["specularIntensity"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Specular Intensity" ),
						&el,
						0.0f,
						1.0f ) );
				}

				if ( auto el = cb["modelSpecularGloss"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Glossiness" ),
						&el,
						1.0f,
						100.0f,
						"%.1f",
						1.5f ) );
				}

				return bDirty;
			}
		} evTerrain;

		accept( evTerrain );
	}
	ImGui::End();
#endif
}

dx::XMMATRIX Terrain::getRotation( const DirectX::XMFLOAT3 &rotIn ) cond_noex
{
	auto &worldTransformMatrix = getTransform();
	DirectX::XMFLOAT4X4 worldTransform{};
	dx::XMStoreFloat4x4( &worldTransform,
		worldTransformMatrix );

	const dx::XMFLOAT3 pitchYawRoll = util::extractEulerAngles( worldTransform );
	const float pitch = std::clamp( pitchYawRoll.x + rotIn.x,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const float yaw = util::wrapAngle( pitchYawRoll.y + rotIn.y );
	const float roll = std::clamp( pitchYawRoll.z + rotIn.z,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const dx::XMFLOAT3 angles{pitch, yaw, roll};

	return dx::XMMatrixRotationRollPitchYaw( angles.x,
		angles.y,
		angles.z );
}