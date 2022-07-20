#pragma once

#include <vector>
#include <memory>


class Camera;
class Graphics;

namespace ren
{
	class Renderer;
}

class CameraManager final
{
	static inline CameraManager* m_pInstance;
	std::vector<std::shared_ptr<Camera>> m_cameras;
	int m_activeCameraIndex = 0;
	int m_controlledCamera = 0;
	std::pair<int,int> m_clientWidthHeight;

	CameraManager() = default;
	Camera& getControlledCamera();
public:
	static CameraManager& getInstance();
	static void resetInstance();
public:
	CameraManager( const CameraManager& rhs ) = delete;
	CameraManager& operator=( const CameraManager& rhs ) = delete;

	void setWidthHeight( int width, int height ) noexcept;
	int getClientWidth() const noexcept;
	int getClientHeight() const noexcept;
	void spawnImguiWindow( Graphics &gph );
	void bind( Graphics &gph );
	void add( std::shared_ptr<Camera> pCam );
	void connectEffectsToRenderer( ren::Renderer& r );
	void render( size_t channels ) const;
	Camera& getActiveCamera() const noexcept;
	std::shared_ptr<Camera> shareActiveCamera() const noexcept;
};
