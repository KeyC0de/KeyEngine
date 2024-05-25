#pragma once

#include <filesystem>
#include <vector>
#include "dynamic_vertex_buffer.h"


struct aiMaterial;
struct aiMesh;
class Graphics;
class Effect;
class VertexBuffer;
class IndexBuffer;

//=============================================================
//	\class	MaterialLoader
//	\author	KeyC0de
//	\date	2021/10/29 21:04
//	\brief	dynamically populate model Bindables & attributes given
//				Assimp::aiMaterial attributes
//=============================================================
class MaterialLoader final
{
	ver::VertexInputLayout m_vertexLayout;
	std::string m_modelPath;
	std::string m_name;
	std::vector<Effect> m_effects;
public:
	enum LightingModel
	{
		BlinnPhong,
		PBR_UE,
	};
public:
	MaterialLoader( Graphics &gfx, const aiMaterial &aimaterial, const std::filesystem::path &modelPath, const LightingModel lightingModel ) cond_noex;

	std::shared_ptr<VertexBuffer> makeVertexBuffer( Graphics &gfx, const aiMesh &aimesh, float scale = 1.0f ) const cond_noex;
	std::shared_ptr<IndexBuffer> makeIndexBuffer( Graphics &gfx, const aiMesh &aimesh ) const cond_noex;
	std::vector<Effect> getEffects() const noexcept;
private:
	std::string calcMeshTag( const aiMesh &mesh ) const noexcept;
	ver::VBuffer makeVertexBuffer_impl( const aiMesh &aimesh ) const noexcept;
	std::vector<unsigned> makeIndexBuffer_impl( const aiMesh &aimesh ) const noexcept;
};