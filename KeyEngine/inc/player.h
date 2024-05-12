#pragma once

#include "mesh.h"


class Camera;

class Player
	: public Mesh
{
	Camera *m_cam;
	float m_3dPersonCamDistance;
	float m_3dPersonCamDistancePrev;
public:
	
};