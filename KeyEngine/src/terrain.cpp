#include "terrain.h"
#include "index_buffer.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "transform_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"
#include "constant_buffer_ex.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "rendering_channel.h"
#include "utils.h"
#include "math_utils.h"
#include "d3d_utils.h"


namespace dx = DirectX;

Terrain::Terrain( Graphics &gfx,
	const int length,
	const int width,
	const std::string &heightMapfilename /*= ""*/,
	const float initialScale /*= 1.0f*/,
	const DirectX::XMFLOAT3 &initialRot /*= {0.0f, 0.0f, 0.0f}*/,
	const DirectX::XMFLOAT3 &initialPos /*= {0.0f, 0.0f, 0.0f}*/,
	const int normalizeAmount /*= 4*/,
	const int terrainAreaUnitMultiplier /*= 10*/ )
	:
	Mesh{{1.0f, 1.0f, 1.0f}, initialRot, initialPos}
{
	const int lengthVerts = util::ceil( length ) * 2;
	const int widthVerts = util::ceil( width ) * 2;

	auto planarGrid = heightMapfilename.empty() ?
		Geometry::makePlanarGridTextured( length, width, lengthVerts, widthVerts ) :
		Geometry::makePlanarGridTexturedFromHeighmap( heightMapfilename, normalizeAmount, terrainAreaUnitMultiplier, length, width, lengthVerts, widthVerts );

	if ( initialScale != 1.0f )
	{
		planarGrid.transform( dx::XMMatrixScaling( initialScale, initialScale, initialScale ) );
	}

	const auto geometryTag = s_geometryTag + std::to_string( length ) + std::to_string( width ) + std::to_string( initialScale );

	m_pVertexBuffer = VertexBuffer::fetch( gfx, geometryTag, planarGrid.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gfx, geometryTag, planarGrid.m_indices );

	createAabb( planarGrid.m_vb );
	setMeshId();

	auto transformVscb = std::make_shared<TransformVSCB>( gfx, 0u );
	{// opaque reflectance effect
		Effect opaque{rch::opaque, "opaque", false};
		opaque.addBindable( transformVscb );

		opaque.addBindable( Texture::fetch( gfx, "assets/models/brick_wall/brick_wall_diffuse.jpg", 0u ) );

		opaque.addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gfx, "plane_vs.cso" );
		opaque.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *pVs ) );
		opaque.addBindable( std::move( pVs ) );

		opaque.addBindable( PixelShader::fetch( gfx, "plane_ps.cso" ) );

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
	{//shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", false};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", false};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", false};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gfx, 0u, 1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "materialColor" );
		auto cb = con::CBuffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT3{1.0f, 0.4f, 0.4f};
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gfx, 0u, cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gfx, planarGrid.m_vb.getLayout(), *VertexShader::fetch( gfx, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
	{// wireframe effect
		Effect wireframe{rch::wireframe, "wireframe", true};

		auto pVs = VertexShader::fetch( gfx, "flat_vs.cso" );
		wireframe.addBindable( InputLayout::fetch( gfx, m_pVertexBuffer->getLayout(), *pVs ) );
		wireframe.addBindable( std::move( pVs ) );

		wireframe.addBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );

		struct ColorPSCB
		{
			dx::XMFLOAT3 color{1.0f, 1.0f, 1.0f};
			float paddingPlaceholder = 0.0f;
		} colorPscb;

		wireframe.addBindable( PixelShaderConstantBuffer<ColorPSCB>::fetch( gfx, colorPscb, 0u ) );
		wireframe.addBindable( std::make_shared<TransformVSCB>( gfx, 0u ) );

		addEffect( std::move( wireframe ) );
	}
}

void Terrain::displayImguiWidgets( Graphics &gfx,
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

		class EVTerrain
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
					dirtyCheck( ImGui::ColorPicker3( tagImGuiWidget( "materialColor" ), reinterpret_cast<float*>( &static_cast<dx::XMFLOAT3&>( el ) ) ) );
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
		} evTerrain;

		accept( evTerrain );
	}
	ImGui::End();
#endif
}

void Terrain::transformVertices( ver::VBuffer &vb,
	const double value ) noexcept
{
	const size_t numVertices = vb.getVertexCount();
	for ( size_t i = 0; i < numVertices; ++i )
	{
		auto &vertexHeight = vb[0].getElement<ver::VertexInputLayout::ILEementType::Position3D>().y;
	}
}