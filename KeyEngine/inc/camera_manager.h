#pragma once

#include <vector>
#include <memory>
#include "non_copyable.h"


class Camera;
class Graphics;

namespace ren
{
	class Renderer;
}

class CameraManager final
	: public NonCopyable
{
	static inline CameraManager *m_pInstance;
	int m_activeCameraIndex = 0;
	int m_controlledCameraIndex = 0;
	unsigned int m_clientWidth;
	unsigned int m_clientHeight;
	std::vector<std::shared_ptr<Camera>> m_cameras;

	CameraManager() = default;
	Camera& controlledCamera();
public:
	static CameraManager& getInstance();
public:
	void setWidth( const int width ) noexcept;
	void setHeight( const int height ) noexcept;
	const int getClientWidth() const noexcept;
	const int getClientHeight() const noexcept;
	void spawnImguiWindow( Graphics &gph );
	void bind( Graphics &gph );
	void add( std::shared_ptr<Camera> pCam );
	void connectEffectsToRenderer( ren::Renderer &r );
	void render( const size_t channels ) const;
	Camera& activeCamera() const noexcept;
	std::shared_ptr<Camera> shareActiveCamera() const noexcept;
};
