#include "cube.h"
#include "geometry.h"
#include "input_layout.h"
#include "pixel_shader.h"
#include "primitive_topology.h"
#include "transform_vscb.h"
#include "transform_scale_vscb.h"
#include "vertex_buffer.h"
#include "vertex_shader.h"
#include "texture.h"
#include "texture_sampler.h"
#include "rasterizer.h"
#include "constant_buffer_ex.h"
#include "imgui.h"
#include "rendering_channel.h"
#include "utils.h"


namespace dx = DirectX;

Cube::Cube( Graphics &gph,
	const dx::XMFLOAT3 &startingPos,
	float scale )
	:
	m_pos{startingPos}
{
	auto cube = GeometryCube::makeIndependentFacesTextured();
	cube.transform( dx::XMMatrixScaling( scale,
		scale,
		scale ) );
	cube.setNormalsIndependent();
	const auto geometryTag = "$cube." + std::to_string( scale );

	m_pVertexBuffer = VertexBuffer::fetch( gph,
		geometryTag,
		cube.m_vb );
	m_pIndexBuffer = IndexBuffer::fetch( gph,
		geometryTag,
		cube.m_indices );
	m_pPrimitiveTopology = PrimitiveTopology::fetch( gph );

	auto transformVscb = std::make_shared<TransformVSCB>( gph,
		0u );
	{// lambertian reflectance effect
		Effect lambertian{rch::lambert, "lambertian", true};
		lambertian.addBindable( transformVscb );

		lambertian.addBindable( Texture::fetch( gph,
			"assets/models/brick_wall/brick_wall_diffuse.jpg",
			0u ) );
		lambertian.addBindable( TextureSampler::fetch( gph,
			0u,
			TextureSampler::FilterMode::Anisotropic,
			TextureSampler::AddressMode::Wrap ) );

		auto pVs = VertexShader::fetch( gph,
			"cube_vs.cso" );
		lambertian.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*pVs ) );
		lambertian.addBindable( std::move( pVs ) );

		lambertian.addBindable( PixelShader::fetch( gph,
			"cube_ps.cso" ) );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float3>( "modelSpecularColor" );
		cbLayout.add<con::Float>( "modelSpecularGloss" );
		auto cb = con::Buffer( std::move( cbLayout ) );
		cb["modelSpecularColor"] = dx::XMFLOAT3{1.0f, 1.0f, 1.0f};
		cb["modelSpecularGloss"] = 20.0f;
		lambertian.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		lambertian.addBindable( Rasterizer::fetch( gph, false ) );

		addEffect( std::move( lambertian ) );
	}
	{// shadow map effect
		Effect shadowMap{rch::shadow, "shadowMap", true};
		shadowMap.addBindable( transformVscb );

		shadowMap.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( shadowMap ) );
	}
	{// blur outline mask effect
		Effect blurOutlineMask{rch::blurOutline, "blurOutlineMask", false};
		blurOutlineMask.addBindable( transformVscb );

		blurOutlineMask.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineMask ) );
	}
	{// blur outline draw effect
		Effect blurOutlineDraw{rch::blurOutline, "blurOutlineDraw", false};
		blurOutlineDraw.addBindable( transformVscb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::Buffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT4{1.0f, 0.4f, 0.4f, 1.0f};
		blurOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		blurOutlineDraw.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( blurOutlineDraw ) );
	}
	{// solid outline mask effect
		Effect solidOutlineMask{rch::solidOutline, "solidOutlineMask", true};
		solidOutlineMask.addBindable( transformVscb );

		solidOutlineMask.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineMask ) );
	}
	{// solid outline draw effect
		Effect solidOutlineDraw{rch::solidOutline, "solidOutlineDraw", true};

		auto transformScaledVcb = std::make_shared<TransformScaleVSCB>( gph,
			0u,
			1.04f );
		solidOutlineDraw.addBindable( transformScaledVcb );

		con::RawLayout cbLayout;
		cbLayout.add<con::Float4>( "materialColor" );
		auto cb = con::Buffer( std::move( cbLayout ) );
		cb["materialColor"] = dx::XMFLOAT4{1.0f, 0.4f, 0.4f, 1.0f};
		solidOutlineDraw.addBindable( std::make_shared<PixelShaderConstantBufferEx>( gph,
			0u,
			cb ) );

		solidOutlineDraw.addBindable( InputLayout::fetch( gph,
			cube.m_vb.getLayout(),
			*VertexShader::fetch( gph, "flat_vs.cso" ) ) );

		addEffect( std::move( solidOutlineDraw ) );
	}
}

void Cube::setWorldPosition( const dx::XMFLOAT3 &pos ) noexcept
{
	this->m_pos = pos;
}

void Cube::setWorldRotation( float roll,
	float pitch,
	float yaw ) noexcept
{
	this->m_roll = roll;
	this->m_pitch = pitch;
	this->m_yaw = yaw;
}

const dx::XMMATRIX Cube::getTransform() const noexcept
{
	return dx::XMMatrixRotationRollPitchYaw( m_roll, m_pitch, m_yaw ) *
		dx::XMMatrixTranslation( m_pos.x, m_pos.y, m_pos.z );
}

void Cube::displayImguiWidgets( Graphics &gph,
	const char *name ) noexcept
{
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
			&m_pitch,
			-90.0f,
			90.0f );
		ImGui::SliderAngle( "Yaw",
			&m_yaw,
			-180.0f,
			180.0f );
		ImGui::SliderAngle( "Roll",
			&m_roll,
			-180.0f,
			180.0f );

		class EVCube
			: public IEffectVisitor
		{
		public:
			void onSetEffect() override
			{
				ImGui::TextColored( {0.4f, 1.0f, 0.6f, 1.0f},
					util::capitalizeFirstLetter( m_pEffect->getTargetPassName() ).c_str() );

				bool active = m_pEffect->isEnabled();
				using namespace std::string_literals;
				ImGui::Checkbox( ( "Effect Active##"s + std::to_string( m_effectId ) ).c_str(),
					&active );
				m_pEffect->setEnabled( active );
			}

			bool onVisit( con::Buffer &cb ) override
			{
				float bDirty = false;
				const auto dirtyCheck = [&bDirty]( bool bChanged )
				{
					bDirty = bDirty || bChanged;
				};
				auto tag = [tagScratch = std::string{},
					tagString = "##" + std::to_string( m_cbId )]
					( const char *label ) mutable
					{
						tagScratch = label + tagString;
						return tagScratch.c_str();
					};

				if ( auto el = cb["scale"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tag( "Scale" ),
						&el,
						1.0f,
						2.0f,
						"%.3f",
						3.5f ) );
				}

				if ( auto el = cb["materialColor"]; el.isValid() )
				{
					dirtyCheck( ImGui::ColorPicker4( tag( "materialColor" ),
						reinterpret_cast<float*>( &static_cast<dx::XMFLOAT4&>( el ) ) ) );
				}

				if ( auto el = cb["specularIntensity"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tag( "Specular Intensity" ),
						&el,
						0.0f,
						1.0f ) );
				}

				if ( auto el = cb["modelSpecularGloss"]; el.isValid() )
				{
					dirtyCheck( ImGui::SliderFloat( tag( "Glossiness" ),
						&el,
						1.0f,
						100.0f,
						"%.1f",
						1.5f ) );
				}

				return bDirty;
			}
		} evCube;

		accept( evCube );
	}
	ImGui::End();
}