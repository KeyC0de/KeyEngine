#include "camera_manager.h"
#include "imgui.h"
#include "camera.h"
#include "graphics.h"


CameraManager& CameraManager::instance()
{
	static CameraManager m_instance{};
	return m_instance;
}

void CameraManager::setWidth( const int width ) noexcept
{
	m_clientWidth = width;
}

void CameraManager::setHeight( const int height ) noexcept
{
	m_clientHeight = height;
}

const int CameraManager::getClientWidth() const noexcept
{
	return m_clientWidth;
}

const int CameraManager::getClientHeight() const noexcept
{
	return m_clientWidth;
}

void CameraManager::spawnImguiWindow( Graphics &gph )
{
	if ( ImGui::Begin( "Cameras" ) )
	{
		if ( ImGui::BeginCombo( "Active", activeCamera().getName().c_str() ) )
		{
			for ( int i = 0; i < std::size( m_cameras ); ++i )
			{
				const bool bSelected = i == m_activeCameraIndex;
				if ( ImGui::Selectable( m_cameras[i]->getName().c_str(), bSelected ) )
				{
					m_activeCameraIndex = i;
				}
			}
			ImGui::EndCombo();
		}

		if ( ImGui::BeginCombo( "Controlled", controlledCamera().getName().c_str() ) )
		{
			for ( int i = 0; i < std::size( m_cameras ); ++i )
			{
				const bool bSelected = i == m_controlledCameraIndex;
				if ( ImGui::Selectable( m_cameras[i]->getName().c_str(), bSelected ) )
				{
					m_controlledCameraIndex = i;
				}
			}
			ImGui::EndCombo();
		}

		controlledCamera().displayImguiWidgets( gph );
	}
	ImGui::End();
}

void CameraManager::bind( Graphics &gph )
{
	gph.setViewMatrix( activeCamera().getViewMatrix() );
}

void CameraManager::add( std::shared_ptr<Camera> pCam )
{
	m_cameras.push_back( std::move( pCam ) );
}

void CameraManager::connectEffectsToRenderer( ren::Renderer &r )
{
	for ( auto &pCam : m_cameras )
	{
		pCam->connectEffectsToRenderer( r );
	}
}

void CameraManager::render( const size_t channels ) const
{
	for ( size_t i = 0; i < m_cameras.size(); ++i )
	{
		if ( i != m_activeCameraIndex )
		{
			m_cameras[i]->render( channels );
		}
	}
}

Camera& CameraManager::activeCamera() const noexcept
{
	return *m_cameras[m_activeCameraIndex];
}

std::shared_ptr<Camera> CameraManager::shareActiveCamera() const noexcept
{
	return m_cameras[m_activeCameraIndex];
}

Camera& CameraManager::controlledCamera()
{
	return *m_cameras[m_controlledCameraIndex];
}