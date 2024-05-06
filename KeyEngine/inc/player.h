#pragma once

#include "mesh.h"


class Player
	: public Mesh
{
	float m_3dPersonCamDistance;
	float m_3dPersonCamDistancePrev;
public:
	using Mesh::Mesh;
};