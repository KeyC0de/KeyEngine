#pragma once

#include <vector>
#include <memory>
#include "non_copyable.h"
#include "rendering_channel.h"


class Camera;
class Graphics;

namespace ren
{
	class Renderer;
}

//=============================================================
//	\class	CameraManager
//	\author	KeyC0de
//	\date	2022/05/03 12:08
//	\brief	singleton class
//=============================================================
class CameraManager final
	: public NonCopyable
{
	static inline CameraManager *s_pInstance;
	int m_activeCameraIndex = 0;
	int m_controlledCameraIndex = 0;
	std::vector<std::shared_ptr<Camera>> m_cameras;
private:
	CameraManager() = default;
public:
	static CameraManager& getInstance();
public:
	void displayImguiWidgets( Graphics &gph ) noexcept;
	void add( std::shared_ptr<Camera> pCam );
	void connectEffectsToRenderer( ren::Renderer &r );
	void render( const size_t channels = rch::all ) const;
	void updateDimensions( Graphics &gph );
	Camera& getActiveCamera() cond_noex;
	const Camera& getActiveCamera() const noexcept;
	std::shared_ptr<Camera> shareActiveCamera() const noexcept;
	Camera& getControlledCamera() cond_noex;
	const Camera& getControlledCamera() const noexcept;
	std::shared_ptr<Camera> shareControlledCamera() const noexcept;
};
