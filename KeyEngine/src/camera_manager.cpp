#include "camera_manager.h"
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "camera.h"
#include "graphics.h"


CameraManager& CameraManager::getInstance()
{
	static CameraManager instance{};
	return instance;
}

void CameraManager::displayImguiWidgets( Graphics &gph ) noexcept
{
#ifndef FINAL_RELEASE
	if ( ImGui::Begin( "Cameras" ) )
	{
		if ( ImGui::BeginCombo( "Active", getActiveCamera().getName().c_str() ) )
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

		if ( ImGui::BeginCombo( "Controlled", getControlledCamera().getName().c_str() ) )
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

		getControlledCamera().displayImguiWidgets( gph );
	}
	ImGui::End();
#endif
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

void CameraManager::updateDimensions( Graphics &gph )
{
	for ( auto& cam : m_cameras )
	{
		cam->updateDimensions( gph );
	}
}

Camera& CameraManager::getActiveCamera() cond_noex
{
	return *m_cameras[m_activeCameraIndex];
}

const Camera& CameraManager::getActiveCamera() const noexcept
{
	return *m_cameras[m_activeCameraIndex];
}

std::shared_ptr<Camera> CameraManager::shareActiveCamera() const noexcept
{
	return m_cameras[m_activeCameraIndex];
}

Camera& CameraManager::getControlledCamera() cond_noex
{
	return *m_cameras[m_controlledCameraIndex];
}

const Camera& CameraManager::getControlledCamera() const noexcept
{
	return *m_cameras[m_controlledCameraIndex];
}

std::shared_ptr<Camera> CameraManager::shareControlledCamera() const noexcept
{
	return m_cameras[m_controlledCameraIndex];
}