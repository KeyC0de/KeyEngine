#include "cube.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "primitive_topology.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#include "blend_state.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "rendering_channel.h"
#include "utils.h"


namespace dx = DirectX;

Cube::Cube( Graphics &gfx,
	const float initialScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT4 &color /*= {1.0f, 0.4f, 0.4f, 1.0f}*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos},
	m_colorPscb{color}
{
	auto cube = Geometry::makeCubeIndependentFacesTextured();
	if ( initialScale != 1.0f )
	{
		cube.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}
	if ( initialRot.x != 0.0f && initialRot.y != 0.0f && initialRot.z != 0.0f )
	{
		cube.transform( dx::XMMatrixRotationRollPitchYaw( initialRot.x, initialRot.y, initialRot.z ) );
	}

	cube.setFlatShadedIndependentNormals();
	const auto geometryTag = s_geometryTag + std::to_string( initialScale );

	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, cube.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, cube.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	createAabb( cube.m_vb );
	setMeshId();

	const std::string diffuseTexturePath{"assets/models/brick_wall/brick_wall_diffuse.jpg"};
	auto transformVscb = std::make_shared<TransformVSCB>( gfx, 0u );
	if ( color.w < 1.0f )
	{// transparent reflectance effect
		Effect transparent{rch::transparent, "transparent", true};
		transparent.addBindable( transformVscb );

		transparent.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		transparent.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

		transparent.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u ) );

		auto pVs = VertexShader::fetch( gfx, "cube_vs.cso" );
		transparent.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *pVs ) );
		transparent.addBindable( std::move( pVs ) );

		transparent.addBindable( PixelShader::fetch( gfx, "cube_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{color.x, color.y, color.z};
		cb["modelSpecularGloss"] = 128.0f;
		transparent.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		transparent.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		transparent.addBindable( std::make_shared<BlendState>( gfx, BlendState::Mode::Alpha, 0u, color.w ) );

		addEffect( std::move( transparent ) );
	}
	else
	{// opaque reflectance effect
		Effect opaque{rch::opaque, "opaque", true};
		opaque.addBindable( transformVscb );

		opaque.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		opaque.addBindable( Texture::fetch( gfx, diffuseTexturePath, 0u ) );

		opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gfx, "cube_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "cube_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
		cb["modelSpecularGloss"] = 128.0f;
		opaque.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		opaque.addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

		addEffect( std::move( opaque ) );
	}
	{// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", true};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( PrimitiveTopology::fetch( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

		shadowMap.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{// blur outline mask effect
		Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", true};
		blurOutlineMask.addBindable( transformVscb );

		blurOutlineMask.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineMask ) );
	}
	{// blur outline draw effect
		Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", true};
		blurOutlineDraw.addBindable( transformVscb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineDraw ) );
	}
	{// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = color;
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, cube.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
}

void Cube::displayImguiWidgets( Graphics &gfx,
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

		class EVCube
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

				if ( auto el = cb["specularIntensity"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Specular Intensity" ), &el, 0.0f, 1.0f ) );
				}

				if ( auto el = cb["modelSpecularGloss"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tagImGuiWidget( "Glossiness" ), &el, 1.0f, 200.0f, "%.1f", 1.5f ) );
				}

				return bDirty;
			}
		} evCube;

		accept( evCube );
	}
	ImGui::End();
#endif
}