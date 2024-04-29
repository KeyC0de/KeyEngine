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

void CameraManager::setWidth( const int width ) noexcept
{
	m_clientWidth = width;
}

void CameraManager::setHeight( const int height ) noexcept
{
	m_clientHeight = height;
}

int CameraManager::getClientWidth() const noexcept
{
	return m_clientWidth;
}

int CameraManager::getClientHeight() const noexcept
{
	return m_clientWidth;
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

		controlledCamera().displayImguiWidgets( gph );
	}
	ImGui::End();
#endif
}

void CameraManager::bind( Graphics &gph )
{
	ASSERT( m_clientWidth != 0 && m_clientHeight != 0, "Invalid camera binding!" );
	gph.setViewMatrix( getActiveCamera().getViewMatrix() );
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

Camera& CameraManager::activeCamera() cond_noex
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

Camera& CameraManager::controlledCamera() cond_noex
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