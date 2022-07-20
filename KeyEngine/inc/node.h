#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>


class Model;
class Drawable;
class IEffectVisitor;
class IModelVisitor;

class Node
{
	friend Model;

	std::string m_name;
	int m_id;	// useful for ImGUI widget rendering stuff
	std::vector<std::unique_ptr<Node>> m_pChildren;
	std::vector<Drawable*> m_pDrawables;
	DirectX::XMFLOAT4X4 m_localTransform;
	DirectX::XMFLOAT4X4 m_worldTransform;
public:
	Node( int id, const std::string &name, std::vector<Drawable*> pDrawables,
		const DirectX::XMMATRIX &localTransform ) cond_noex;

	void update( float dt, const DirectX::XMMATRIX &parentWorldTransform ) const cond_noex;
	void render( size_t channels ) const cond_noex;
	void setTransform( const DirectX::XMMATRIX &worldTransform ) noexcept;
	const DirectX::XMFLOAT4X4& getWorldTransform() const noexcept;
	int getId() const noexcept;
	bool hasChildren() const noexcept;
	void accept( IModelVisitor& mv );
	void accept( IEffectVisitor& ev );
	const std::string &getName() const noexcept;
private:
	void addChild( std::unique_ptr<Node> pChild ) cond_noex;
};