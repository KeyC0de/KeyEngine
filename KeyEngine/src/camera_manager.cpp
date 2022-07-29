#include "camera_manager.h"
#include "imgui.h"
#include "camera.h"
#include "graphics.h"


CameraManager &CameraManager::getInstance()
{
	if ( m_pInstance == nullptr )
	{
		m_pInstance = new CameraManager{};
	}
	return *m_pInstance;
}

void CameraManager::resetInstance()
{
	if ( m_pInstance != nullptr )
	{
		delete m_pInstance;
	}
}

void CameraManager::setWidthHeight( int width,
	int height ) noexcept
{
	m_clientWidthHeight.first = width;
	m_clientWidthHeight.second = height;
}

int CameraManager::getClientWidth() const noexcept
{
	return m_clientWidthHeight.first;
}

int CameraManager::getClientHeight() const noexcept
{
	return m_clientWidthHeight.second;
}

void CameraManager::spawnImguiWindow( Graphics &gph )
{
	if ( ImGui::Begin( "Cameras" ) )
	{
		if ( ImGui::BeginCombo( "Active",
			getActiveCamera().getName().c_str() ) )
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
		
		if ( ImGui::BeginCombo( "Controlled",
			getControlledCamera().getName().c_str() ) )
		{
			for ( int i = 0; i < std::size( m_cameras ); ++i )
			{
				const bool bSelected = i == m_controlledCamera;
				if ( ImGui::Selectable( m_cameras[i]->getName().c_str(), bSelected ) )
				{
					m_controlledCamera = i;
				}
			}
			ImGui::EndCombo();
		}

		getControlledCamera().displayImguiWidgets( gph );
	}
	ImGui::End();
}

void CameraManager::bind( Graphics &gph )
{
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

void CameraManager::render( size_t channels ) const
{
	for ( size_t i = 0; i < m_cameras.size(); ++i )
	{
		if ( i != m_activeCameraIndex )
		{
			m_cameras[i]->render( channels );
		}
	}
}

Camera &CameraManager::getActiveCamera() const noexcept
{
	return *m_cameras[m_activeCameraIndex];
}

std::shared_ptr<Camera> CameraManager::shareActiveCamera() const noexcept
{
	return m_cameras[m_activeCameraIndex];
}

Camera &CameraManager::getControlledCamera()
{
	return *m_cameras[m_controlledCamera];
}
