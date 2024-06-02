#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>


class Model;
class Mesh;
class IImGuiConstantBufferVisitor;
class IImguiNodeVisitor;

class Node
{
	friend class Model;
	//friend std::unique_ptr<Node> std::make_unique<Node>();

	bool m_bWorldTransformNeedsUpdate = true;
	int m_imguiId;
	std::string m_name;
	DirectX::XMFLOAT3 m_scale;
	DirectX::XMFLOAT3 m_rot;		// {x = pitch, y = yaw, y = roll} (in radians)
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_scalePrev;	// previous* are updated with current* values only on update()
	DirectX::XMFLOAT3 m_rotPrev;
	DirectX::XMFLOAT3 m_posPrev;
	DirectX::XMFLOAT4X4 m_localTransform;// the transform of this Node/Mesh relative to its parent; stored in row major upfront, in the Node's ctor, taken from FBX importer, shouldn't change during the course of the program
	DirectX::XMFLOAT4X4 m_worldTransform;// the transform of this Node/Mesh in world space; stored in row major, transposed in TransformVSCB b4 lead to HLSL
	// The mesh is a property of the node, and its vertices are transformed using the node's transformation matrix. The mesh itself does not have an independent transformation matrix. Instead, it relies on the transformation matrix of the node to which it is attached.
	Node* m_pParent;
	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<Mesh*> m_meshes;	// a node can technically have multiple meshes; however, typically, it has only a single Mesh/geometric-entity attached to it; this is just common practice & not a strict rule
private:
	friend void swap( Node &lhs, Node &rhs );

	void update( const float dt, const DirectX::XMMATRIX &parentBuiltTransform, const float renderFrameInterpolation, const bool bEnableSmoothMovement = false ) cond_noex;
	void addChild( std::unique_ptr<Node> pChild ) cond_noex;
public:
	Node( Node *pParent, const int imguiId, const std::string &name, const DirectX::XMMATRIX &localTransform, std::vector<Mesh*> meshes );
	~Node() noexcept;
	Node( const Node &rhs ) = delete;
	Node& operator=( const Node &rhs ) = delete;
	Node( Node &&rhs ) noexcept;
	Node& operator=( Node &&rhs ) noexcept;

	// transformation functions
	void setWorldTransform( const DirectX::XMFLOAT4X4 &worldTransform ) cond_noex;
	void setWorldTransform( const DirectX::XMMATRIX &worldTransform ) cond_noex;
	void setWorldTransform( const float scale, const DirectX::XMFLOAT3 &rotAnglesRadians, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setWorldTransform( const float scale, const DirectX::XMFLOAT4 &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setWorldTransform( const float scale, const DirectX::XMVECTOR &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setScale( const float scale ) cond_noex;
	void setScale( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void scaleRel( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void setRotation( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void rotateRel( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void setTranslation( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void updateWorldTransform( const float dt, const float renderFrameInterpolation, const bool bEnableSmoothMovement = false ) cond_noex;
	DirectX::XMMATRIX getWorldTransform() const noexcept;
	DirectX::XMFLOAT4X4 getWorldTransform4x4() const noexcept;
	float getScale() const noexcept;
	DirectX::XMFLOAT3 getScale3() const noexcept;
	//	\function	getRotation	||	\date	2024/04/20 18:12
	//	\brief	return Euler Angles {pitch,yaw,roll}
	DirectX::XMFLOAT3 getRotation() const noexcept;
	DirectX::XMVECTOR getRotationQuat() const noexcept;
	DirectX::XMFLOAT3 getPosition() const noexcept;

	void accept( IImguiNodeVisitor &mv );
	void accept( IImGuiConstantBufferVisitor &ev );
	int getImguiId() const noexcept;
	bool hasChildren() const noexcept;
	const std::string& getName() const noexcept;
	bool isRoot() const noexcept;
};
