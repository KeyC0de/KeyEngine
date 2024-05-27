#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>


class Mesh;
class IImGuiMaterialVisitor;
class IImguiNodeVisitor;

class Node
{
	friend class Model;

	std::string m_name;
	int m_imguiId;
	DirectX::XMFLOAT4X4 m_localTransform;		// stored in row major upfront, in the Node's ctor, taken from importer transform, should be equal to DirectX::XMMatrixIdentity() with proper pivots #TODO: test not storing m_localTransform and use DirectX::XMMatrixIdentity instead
	DirectX::XMFLOAT4X4 m_worldTransform;		// stored in row major, transposed in TransformVSCB b4 lead to HLSL
	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<Mesh*> m_meshes;
public:
	Node( const int imguiId, const std::string &name, const DirectX::XMMATRIX &localTransform, std::vector<Mesh*> pmeshes ) cond_noex;

	void update( const float dt, const DirectX::XMMATRIX &parentWorldTransform, const float renderFrameInterpolation ) const cond_noex;
	void render( const size_t channels ) const cond_noex;
	void setMaterialEnabled( const size_t channels, const bool bEnabled ) noexcept;
	int getImguiId() const noexcept;
	bool hasChildren() const noexcept;
	void accept( IImguiNodeVisitor &mv );
	void accept( IImGuiMaterialVisitor &ev );
	const std::string& getName() const noexcept;
	void setWorldTransform( const DirectX::XMFLOAT4X4 &worldTransform ) noexcept;
	void setWorldTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	DirectX::XMMATRIX getWorldTransform() const noexcept;
	DirectX::XMFLOAT4X4& getWorldTransformAccess();
private:
	void addChild( std::unique_ptr<Node> pChild ) cond_noex;
};