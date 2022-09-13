#pragma once

#include <unordered_map>
#include <string>


class Model;
class Node;

class IModelVisitor
{
public:
	virtual ~IModelVisitor() noexcept = default;

	virtual bool visit( class Node &node ) = 0;
	virtual void onVisited( class Node &node ) = 0;
};

class MV
	: IModelVisitor
{
	struct TransformData
	{
		float pitch = 0.0f;
		float yaw = 0.0f;
		float roll = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	Node *m_pSelectedNode = nullptr;
	std::string m_name;
	std::unordered_map<int, TransformData> m_nodeMapTransforms;
public:
	MV( const std::string &name );

	void spawnModelImgui( Model &model );
	const std::string& getName() const noexcept;
private:
	bool visit( Node &node ) override;
	void onVisited( Node &node ) override;
	TransformData& calcTransform() noexcept;
};