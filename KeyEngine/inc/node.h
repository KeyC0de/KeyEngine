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

	bool m_bTransformNeedsUpdate = true;
	int m_imguiId;
	std::string m_name;
	DirectX::XMFLOAT3 m_scale;
	DirectX::XMFLOAT3 m_rot;		// {x = pitch, y = yaw, y = roll} (in radians)
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_scalePrev;
	DirectX::XMFLOAT3 m_rotPrev;
	DirectX::XMFLOAT3 m_posPrev;
	DirectX::XMFLOAT4X4 m_localTransform;
	DirectX::XMFLOAT4X4 m_worldTransform;
	Node* m_pParent;
	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<Mesh*> m_meshes;	// a node can technically have multiple meshes; however, typically, it has only a single Mesh/geometric-entity attached to it; this is just common practice & not a strict rule
private:
	void update( const float dt, const DirectX::XMMATRIX &parentWorldTransform, const float renderFrameInterpolation, const bool bEnableSmoothMovement = false ) cond_noex;
	void addChild( std::unique_ptr<Node> pChild ) cond_noex;
	void invalidateChildrenTransform() noexcept;
	void setWorldTransform( const DirectX::XMMATRIX &worldTransform ) cond_noex;
public:
	Node( Node *pParent, const int imguiId, const std::string &name, const DirectX::XMMATRIX &localTransform, std::vector<Mesh*> meshes );
	~Node() noexcept;
	Node( const Node &rhs ) = delete;
	Node& operator=( const Node &rhs ) = delete;
	Node( Node &&rhs ) noexcept;
	Node& operator=( Node &&rhs ) noexcept;

	friend void swap( Node &lhs, Node &rhs );

	// transformation functions
	void setTransform( const float scale, const DirectX::XMFLOAT3 &rotAnglesRadians, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMFLOAT4 &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setTransform( const float scale, const DirectX::XMVECTOR &rotQuat, const DirectX::XMFLOAT3 &pos ) cond_noex;
	void setScale( const float scale ) cond_noex;
	void setScale( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void scaleRel( const DirectX::XMFLOAT3 &scale ) cond_noex;
	void setRotation( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void rotateRel( const DirectX::XMFLOAT3 &rotAnglesRadians ) cond_noex;
	void setTranslation( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void translateRel( const DirectX::XMFLOAT3 &pos ) cond_noex;
	void updateLocalTransform( const float dt, const float renderFrameInterpolation, const bool bEnableSmoothMovement = false ) cond_noex;
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
