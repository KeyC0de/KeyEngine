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
//
//	\author	KeyC0de
//	\date	2021/10/29 21:04
//
//	\brief	dynamically populate model Bindables & attributes given
//				Assimp::aiMaterial attributes
//			m_vertexLayout is automatically populated in the Lambertian Reflectance Effect
//=============================================================
class MaterialLoader final
{
	ver::VertexLayout m_vertexLayout;
	std::vector<Effect> m_effects;
	std::string m_modelPath;
	std::string m_name;
public:
	MaterialLoader( Graphics &gph, const aiMaterial &aimaterial,
		const std::filesystem::path &modelPath ) cond_noex;

	ver::Buffer extractVertexBuffer( const aiMesh &aimesh ) const noexcept;
	std::shared_ptr<VertexBuffer> makeVertexBuffer( Graphics &gph, const aiMesh &aimesh,
		float scale = 1.0f ) const cond_noex;
	std::shared_ptr<IndexBuffer> makeIndexBuffer( Graphics &gph,
		const aiMesh &aimesh ) const cond_noex;
	std::vector<Effect> getEffects() const noexcept;
private:
	std::string makeMeshTag( const aiMesh &mesh ) const noexcept;
	std::vector<unsigned> extractIndexBuffer( const aiMesh &aimesh ) const noexcept;
};