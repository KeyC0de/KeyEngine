#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>


class Mesh;
class IEffectVisitor;
class IModelVisitor;

class Node
{
	friend class Model;

	std::string m_name;
	int m_imguiId;
	DirectX::XMFLOAT4X4 m_localTransform;		// stored in column major upfront, in the Node's ctor
	DirectX::XMFLOAT4X4 m_worldTransform;		// stored in row major, transposed in TransformVSCB b4 lead to HLSL
	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<Mesh*> m_meshes;
public:
	Node( const int imguiId, const std::string &name, const DirectX::XMMATRIX &localTransform, std::vector<Mesh*> pmeshes ) cond_noex;

	void update( const float dt, const DirectX::XMMATRIX &parentWorldTransform ) const cond_noex;
	void render( const size_t channels ) const cond_noex;
	void setEffectEnabled( const size_t channels, const bool bEnabled ) noexcept;
	const int getImguiId() const noexcept;
	bool hasChildren() const noexcept;
	void accept( IModelVisitor &mv );
	void accept( IEffectVisitor &ev );
	const std::string& getName() const noexcept;
	void setWorldTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	const DirectX::XMFLOAT4X4& getWorldTransform() const noexcept;
	DirectX::XMFLOAT4X4& worldTransform();											// 2 users
	void setCulled( const bool bCulled );
private:
	void addChild( std::unique_ptr<Node> pChild ) cond_noex;
};