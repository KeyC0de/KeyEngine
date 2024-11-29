#include "camera.h"
#include "graphics.h"
#include "camera_widget.h"
#include "camera_frustum.h"
#include "math_utils.h"
#include "geometry.h"
#include "vertex_buffer.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#endif
#include "d3d_utils.h"


namespace dx = DirectX;

DirectX::XMMATRIX Camera::getShadowOrthographicMatrix( const unsigned w,
	const unsigned h,
	const float shadowCamFarZ,
	const float shadowCamNearZ /*= 1.0f*/ ) noexcept
{
	return dx::XMMatrixOrthographicLH( static_cast<float>( w ), static_cast<float>( h ), shadowCamNearZ, shadowCamFarZ );
}

DirectX::XMMATRIX Camera::getShadowProjectionMatrix( const float shadowCamFarZ,
	const float shadowCamNearZ /*= 1.0f*/ ) noexcept
{
	static constexpr auto r = util::PI / 2.0f;
	return dx::XMMatrixPerspectiveFovLH( r, 1.0f, shadowCamNearZ, shadowCamFarZ );
}

DirectX::XMVECTOR Camera::computeTargetVector( const DirectX::XMFLOAT3 &pos,
	const float pitchDeg,
	const float yawDeg ) noexcept
{
	const dx::XMVECTOR lightPosition = dx::XMVectorSet( pos.x, pos.y, pos.z, 1.0f );

	const float pitchRadians = util::toRadians( pitchDeg );
	const float yawRadians = util::toRadians( yawDeg );

	// Calculate the direction vector from pitch and yaw using trigonometry
	const float x = std::cos( yawRadians ) * cos( pitchRadians );	// horizontal component - yaw affects X and Z
	const float y = std::sin( pitchRadians );							// vertical component - pitch affects Y
	const float z = std::sin( yawRadians ) * cos( pitchRadians );	// depth component - yaw affects Z

	const dx::XMVECTOR lightDirection = dx::XMVectorSet( x, y, z, 0.0f );
	// add this direction vector to the light's position to get the point it's looking at (target)
	return dx::XMVectorAdd( lightPosition, lightDirection );
}

std::pair<float,float> Camera::computePitchYawInDegFromDirectionVector( const DirectX::XMFLOAT3 &directionNormalized )
{
	ASSERT( util::isNormalized( directionNormalized ), "Provided direction vector is not normalized!" );

	const float pitchRadians = std::atan2f( directionNormalized.y, std::sqrt(directionNormalized.x * directionNormalized.x + directionNormalized.z * directionNormalized.z ) );								// pitch /in [-pi/2, pi/2] radians
	const float yawRadians = std::atan2f( directionNormalized.x, directionNormalized.z );		// yaw /in [-pi, pi] radians
	return {util::toDegrees( pitchRadians ), util::toDegrees( yawRadians )};
}

DirectX::XMFLOAT3 Camera::computePitchYawRollInDegFromDirectionVector( const DirectX::XMFLOAT3 &directionNormalized )
{
	const auto pitchAndYawDeg = computePitchYawInDegFromDirectionVector( directionNormalized );
	return {pitchAndYawDeg.first, pitchAndYawDeg.second, 0.0f};
}

Camera::Camera( Graphics &gfx,
	const float width,
	const float height,
	const float fovDegrees /*= 90.0f*/,
	const DirectX::XMFLOAT3 &posDefault /*= {0.0f, 0.0f, 0.0f}*/,
	const float pitchDegDefault /*= 0.0f*/,
	const float yawDegDefault /*= 0.0f*/,
	const bool bTethered /*= false*/,
	const bool bPerspectiveProjection /*= true*/,
	const float nearZ /*= 0.5f*/,
	const float farZ /*= 200.0f*/,
	const DirectX::XMFLOAT4 camWidgetColor /*= {0.2f, 0.2f, 0.6f, 1.0f}*/,
	const DirectX::XMFLOAT4 camFrustumColor /*= {0.6f, 0.2f, 0.2f, 1.0f}*/,
	const float translationSpeed /*= 16.0f*/,
	const float rotationSpeed /*= 0.096f*/,
	const bool bShowDebugMeshes /*= false*/ ) noexcept
	:
	m_translationSpeed{translationSpeed},
	m_rotationSpeed{rotationSpeed},
	m_nearZ(nearZ),
	m_farZ(farZ),
	m_NearZDefault(nearZ),
	m_FarZDefault(farZ),
	m_width(width),
	m_height(height),
	m_widthDefault(width),
	m_heightDefault(height),
	m_aspectRatio(static_cast<float>(width) / height),
	m_fovRadians{dx::XMConvertToRadians( fovDegrees )},
	m_posDefault(posDefault),
	m_pitchDefault(util::toRadians( pitchDegDefault )),
	m_yawDefault(util::toRadians( yawDegDefault )),
	m_bTethered(bTethered),
	m_bPespectiveProjection(bPerspectiveProjection),
	m_cameraWidget(std::make_unique<CameraWidget>(gfx, 1.0f, camWidgetColor), gfx, {pitchDegDefault, yawDegDefault, 0.0f}, posDefault),
	m_cameraFrustum(std::make_unique<CameraFrustum>(gfx, 1.0f, camFrustumColor, 1.0f, height, nearZ, farZ), gfx, {yawDegDefault, yawDegDefault, 0.0f}, posDefault),
	m_bShowWidget{bShowDebugMeshes},
	m_bShowFrustum{bShowDebugMeshes}
{
	static int s_id = 0;
	s_id++;
	m_name = std::string{"Camera#"} + std::to_string( s_id );

	if ( m_bTethered )
	{
		m_pos = posDefault;
	}
	resetToDefault( gfx );
}

void Camera::update( const float dt,
	const float lerpBetweenFrames,
	const bool bEnableSmoothMovement /*= false*/ ) cond_noex
{

}

void Camera::render( const size_t channels /*= rch::all*/ ) const cond_noex
{
	if ( m_bShowWidget )
	{
		m_cameraWidget.render( channels );
	}
	if ( m_bShowFrustum )
	{
		m_cameraFrustum.render( channels );
	}
}

void Camera::connectMaterialsToRenderer( ren::Renderer &ren )
{
	m_cameraWidget.connectMaterialsToRenderer( ren );
	m_cameraFrustum.connectMaterialsToRenderer( ren );
}

void Camera::makeActive( Graphics &gfx ) const
{
	gfx.setViewMatrix( getViewMatrix() );
	gfx.setProjectionMatrix( m_bPespectiveProjection ?
		getPerspectiveProjectionMatrix() :
		getOrthographicProjectionMatrix( m_width, m_height ) );
		
}

void Camera::resetToDefault( Graphics &gfx ) noexcept
{
	if ( !m_bTethered )
	{
		m_pos = m_posDefault;
		m_cameraWidget.setTranslation( m_pos );
		m_cameraFrustum.setTranslation( m_pos );
	}

	m_pitch = m_pitchDefault;
	m_yaw = m_yawDefault;

	const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
	m_cameraWidget.setRotation( angles );
	m_cameraFrustum.setRotation( angles );
	m_width = m_widthDefault;
	m_height = m_heightDefault;
	m_nearZ = m_NearZDefault;
	m_farZ = m_FarZDefault;
	updateCameraFrustum( gfx );
}

DirectX::XMMATRIX Camera::getViewMatrix() const noexcept
{
	const auto camPosition = dx::XMLoadFloat3( &m_pos );
	const auto camTarget = getTarget();
	return dx::XMMatrixLookAtLH( camPosition, camTarget, getUp() );
}

DirectX::XMMATRIX Camera::getReflectionViewMatrix( const dx::XMVECTOR &mirrorPlane ) const noexcept
{
	dx::XMMATRIX R = dx::XMMatrixReflect( mirrorPlane );
	return dx::XMMatrixMultiply( getViewMatrix(), R );
}

DirectX::XMMATRIX Camera::getProjectionMatrix( Graphics &gfx,
	const bool bForShadows,
	const float shadowCamFarZ ) const noexcept
{
	return m_bPespectiveProjection ?
		( bForShadows ? getShadowProjectionMatrix( shadowCamFarZ ) : getPerspectiveProjectionMatrix() )
		: ( bForShadows ? getShadowOrthographicMatrix( m_width, m_height, shadowCamFarZ ) : getOrthographicProjectionMatrix( m_width, m_height ) );
}

DirectX::XMVECTOR Camera::getDirection() const noexcept
{
	const dx::XMVECTOR forwardVector{0.0f, 0.0f, 1.0f, 0.0f};
	return dx::XMVector3Transform( forwardVector, getRotationMatrix() );
}

DirectX::XMVECTOR Camera::getRight() const noexcept
{
	return dx::XMVector3Cross( getUp(), getDirection() );
}

DirectX::XMVECTOR Camera::getUp() const noexcept
{
	return dx::XMVECTOR{0.0f, 1.0f, 0.0f, 0.0f};	// aka upVector
}

float Camera::getFovRadians() const noexcept
{
	return m_fovRadians;
}

const std::string& Camera::getName() const noexcept
{
	return m_name;
}

std::vector<dx::XMFLOAT4> Camera::getFrustumPlanes() const noexcept
{
	// x, y, z, and w represent A, B, C and D in the plane equation
	// where ABC are the xyz of the planes normal, and D is the plane constant
	std::vector<dx::XMFLOAT4> frustumPlanes( 6 );

	dx::XMFLOAT4X4 viewProj{};	// Left-Handed
	dx::XMStoreFloat4x4( &viewProj, getViewMatrix() * getPerspectiveProjectionMatrix() );

	// .x|y|z|w of the plane represent A|B|C|D of the plane equation

	// Left Frustum Plane
	// add first column of the matrix to the fourth column
	frustumPlanes[0].x = viewProj._14 + viewProj._11;
	frustumPlanes[0].y = viewProj._24 + viewProj._21;
	frustumPlanes[0].z = viewProj._34 + viewProj._31;
	frustumPlanes[0].w = viewProj._44 + viewProj._41;

	// Right Frustum Plane
	// subtract first column of matrix from the fourth column
	frustumPlanes[1].x = viewProj._14 - viewProj._11;
	frustumPlanes[1].y = viewProj._24 - viewProj._21;
	frustumPlanes[1].z = viewProj._34 - viewProj._31;
	frustumPlanes[1].w = viewProj._44 - viewProj._41;

	// Top Frustum Plane
	// subtract second column of matrix from the fourth column
	frustumPlanes[2].x = viewProj._14 - viewProj._12;
	frustumPlanes[2].y = viewProj._24 - viewProj._22;
	frustumPlanes[2].z = viewProj._34 - viewProj._32;
	frustumPlanes[2].w = viewProj._44 - viewProj._42;

	// Bottom Frustum Plane
	// add second column of the matrix to the fourth column
	frustumPlanes[3].x = viewProj._14 + viewProj._12;
	frustumPlanes[3].y = viewProj._24 + viewProj._22;
	frustumPlanes[3].z = viewProj._34 + viewProj._32;
	frustumPlanes[3].w = viewProj._44 + viewProj._42;

	// Near Frustum Plane
	// we could add the third column to the fourth column to get the near plane, but we don't have to do this because the third column IS the near plane
	frustumPlanes[4].x = viewProj._13;
	frustumPlanes[4].y = viewProj._23;
	frustumPlanes[4].z = viewProj._33;
	frustumPlanes[4].w = viewProj._43;

	// Far Frustum Plane
	// subtract third column of matrix from the fourth column
	frustumPlanes[5].x = viewProj._14 - viewProj._13;
	frustumPlanes[5].y = viewProj._24 - viewProj._23;
	frustumPlanes[5].z = viewProj._34 - viewProj._33;
	frustumPlanes[5].w = viewProj._44 - viewProj._43;

	// Normalize plane normals (A, B and C (xyz))
	// take note that the planes face inward the frustum
	for ( int i = 0; i < 6; ++i )
	{
		const float planeNormalLength = sqrt( ( frustumPlanes[i].x * frustumPlanes[i].x ) + ( frustumPlanes[i].y * frustumPlanes[i].y ) + ( frustumPlanes[i].z * frustumPlanes[i].z ) );
		frustumPlanes[i].x /= planeNormalLength;
		frustumPlanes[i].y /= planeNormalLength;
		frustumPlanes[i].z /= planeNormalLength;
		frustumPlanes[i].w /= planeNormalLength;
	}

	return frustumPlanes;
}

void Camera::displayImguiWidgets( Graphics &gfx ) noexcept
{
#ifndef FINAL_RELEASE
	bool projDirty = false;
	bool rotDirty = false;
	bool posDirty = false;
	const auto dirtyCheck = []( bool d, bool &flag )
	{
		flag = flag || d;
	};

	ImGui::Text( "Projection" );
	dirtyCheck( ImGui::SliderFloat( "Width", &m_width, 0.01f, 4.0f, "%.2f", 1.5f ), projDirty );
	dirtyCheck( ImGui::SliderFloat( "Height", &m_height, 0.01f, 4.0f, "%.2f", 1.5f ), projDirty );
	dirtyCheck( ImGui::SliderFloat( "Near Z", &m_nearZ, 0.01f, m_farZ - 0.01f, "%.2f", 4.0f ), projDirty );
	dirtyCheck( ImGui::SliderFloat( "Far Z", &m_farZ, m_nearZ + 0.01f, 1000.0f, "%.2f", 4.0f ), projDirty );

	ImGui::Text( "Rotation" );
	// set m_pitch to 99.5% of +-90 degrees max to prevent euler angle gimbal lock
	dirtyCheck( ImGui::SliderAngle( "Pitch", &m_pitch, 0.995f * -90.0f, 0.995f * 90.0f ), rotDirty );
	dirtyCheck( ImGui::SliderAngle( "Yaw", &m_yaw, -180.0f, 180.0f ), rotDirty );

	// if the camera is tethered disable its imgui position controls
	if ( !m_bTethered )
	{
		ImGui::Text( "Position" );
		dirtyCheck( ImGui::SliderFloat( "X", &m_pos.x, -80.0f, 80.0f, "%.1f" ), posDirty );
		dirtyCheck( ImGui::SliderFloat( "Y", &m_pos.y, -80.0f, 80.0f, "%.1f" ), posDirty );
		dirtyCheck( ImGui::SliderFloat( "Z", &m_pos.z, -80.0f, 80.0f, "%.1f" ), posDirty );
	}

	ImGui::Checkbox( "Camera Widget", &m_bShowWidget );
	ImGui::Checkbox( "Camera Frustum", &m_bShowFrustum );

	if ( ImGui::Button( "Reset" ) )
	{
		resetToDefault( gfx );
	}

	if ( projDirty )
	{
		updateCameraFrustum( gfx );
	}
	if ( rotDirty )
	{
		const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
		m_cameraWidget.setRotation( angles );
		m_cameraFrustum.setRotation( angles );
	}
	if ( posDirty )
	{
		m_cameraWidget.setTranslation( m_pos );
		m_cameraFrustum.setTranslation( m_pos );
	}
#endif
}

void Camera::onWindowResize( Graphics &gfx )
{
	// #TODO: probably not needed cause camera dimensions are not necessarily tied to gfx dimensions
	//m_width = static_cast<float>( gfx.getClientWidth() );
	//m_height = static_cast<float>( gfx.getClientHeight() );
	//updateCameraFrustum( gfx );
}

void Camera::setTethered( const bool bTethered ) cond_noex
{
	m_bTethered = bTethered;
}

void Camera::setRotationSpeed( const float rotationSpeed ) noexcept
{
	m_rotationSpeed = rotationSpeed;
}

float Camera::getRotationSpeed() const noexcept
{
	return m_rotationSpeed;
}

void Camera::setRotation( const DirectX::XMFLOAT3 &rot ) noexcept
{
	m_pitch = rot.x;
	m_yaw = rot.y;
	const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
	m_cameraWidget.setRotation( angles );
	m_cameraFrustum.setRotation( angles );
}

void Camera::rotateRel( const float dx,
	const float dy ) noexcept
{
	if ( m_bTethered )
	{
		return;
	}

	m_pitch = std::clamp( m_pitch + dy * m_rotationSpeed, 0.995f * -util::PI / 2.0f, 0.995f * util::PI / 2.0f );
	m_yaw = util::wrapAngle( m_yaw + dx * m_rotationSpeed );

	const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
	m_cameraWidget.setRotation( angles );
	m_cameraFrustum.setRotation( angles );
}

void Camera::translateRel( DirectX::XMFLOAT3 translation ) noexcept
{
	if ( m_bTethered )
	{
		return;
	}
	m_posPrev = m_pos;

	dx::XMStoreFloat3( &translation, dx::XMVector3Transform( dx::XMLoadFloat3( &translation ), dx::XMMatrixScaling( m_translationSpeed, m_translationSpeed, m_translationSpeed ) * getRotationMatrix() ) );
	m_pos = {m_pos.x + translation.x, m_pos.y + translation.y, m_pos.z + translation.z};
	m_cameraWidget.setTranslation( m_pos );
	m_cameraFrustum.setTranslation( m_pos );
}

void Camera::setTranslation( const DirectX::XMFLOAT3 &pos ) noexcept
{
	if ( m_bTethered )
	{
		return;
	}

	this->m_pos = pos;
	m_cameraWidget.setTranslation( pos );
	m_cameraFrustum.setTranslation( pos );
}

const DirectX::XMFLOAT3& Camera::getPosition() const noexcept
{
	return m_pos;
}

DirectX::XMFLOAT3& Camera::getPosition()
{
	return m_pos;
}

DirectX::XMFLOAT3 Camera::getRotation() const noexcept
{
	return dx::XMFLOAT3{m_pitch, m_yaw, 0};
}

DirectX::XMMATRIX Camera::getPositionMatrix() const noexcept
{
	return dx::XMMatrixTranslation( m_pos.x, m_pos.y, m_pos.z );
}

DirectX::XMMATRIX Camera::getRotationMatrix() const noexcept
{
	return dx::XMMatrixRotationRollPitchYaw( m_pitch, m_yaw, 0.0f );
}

DirectX::XMVECTOR Camera::getTarget() const noexcept
{
	const auto camPosition = dx::XMLoadFloat3( &m_pos );
	const auto lookVector = getDirection();
	return dx::XMVectorAdd( camPosition, lookVector );
}

void Camera::updateCameraFrustum( Graphics &gfx )
{
	auto g = geometry::makeCameraFrustum( m_width, m_height, m_nearZ, m_farZ );
	auto *frustumMesh = m_cameraFrustum.getMesh();
	frustumMesh->getVertexBuffer() = std::make_shared<VertexBuffer>( gfx, g.m_vb );
	frustumMesh->createAabb( g.m_vb );
	m_aspectRatio = m_width / m_height;
}

DirectX::XMMATRIX Camera::getOrthographicProjectionMatrix( const unsigned width,
	const unsigned height ) const noexcept
{
	return dx::XMMatrixOrthographicLH( static_cast<float>( width ), static_cast<float>( height ), m_nearZ, m_farZ );
}

DirectX::XMMATRIX Camera::getPerspectiveProjectionMatrix() const noexcept
{
	return dx::XMMatrixPerspectiveFovLH( m_fovRadians, m_aspectRatio, m_nearZ, m_farZ );
}
