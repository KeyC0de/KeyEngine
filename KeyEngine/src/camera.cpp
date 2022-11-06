#include "camera.h"
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "graphics.h"
#include "math_utils.h"


namespace dx = DirectX;

DirectX::XMMATRIX Camera::getShadowOrthographicMatrix( const unsigned w,
	const unsigned h ) noexcept
{
	return dx::XMMatrixOrthographicLH( (float) w,
		(float) h,
		1.0f,
		100.0f );
}

DirectX::XMMATRIX Camera::getShadowProjectionMatrix( const float farZ ) noexcept
{
	// Shadows farZ hardcoded at 100units for now
	static constexpr auto r = util::PI / 2.0f;
	return dx::XMMatrixPerspectiveFovLH( r,
		1.0f,
		1.0f,
		100.0f );
}

Camera::Camera( Graphics &gph,
	const std::string &name,
	const int width,
	const int height,
	const float fovDegrees,
	const DirectX::XMFLOAT3 &homePos,
	const float homePitch,
	const float homeYaw,
	const bool bTethered,
	const float nearZ,
	const float farZ ) noexcept
	:
	m_name(name),
	m_aspectRatio(static_cast<float>(width) / height),
	m_fovRadians{dx::XMConvertToRadians( fovDegrees )},
	m_homePosition(homePos),
	m_homePitch(homePitch),
	m_homeYaw(homeYaw),
	m_width(1.0f),
	m_height((static_cast<float>(height) / util::gcd(width, height)) /
		(static_cast<float>(width) / util::gcd(width, height))),
	m_homeWidth(m_width),
	m_homeHeight(m_height),
	m_homeNearZ(m_nearZ),
	m_homeFarZ(m_farZ),
	m_widget(gph),
	m_cameraFrustum(gph, 1.0f, (static_cast<float>(height) / util::gcd(width, height)) /
			(static_cast<float>(width) / util::gcd(width, height)),
		m_nearZ, m_farZ),
	m_bTethered(bTethered),
	m_nearZ(nearZ),
	m_farZ(farZ)
{
	if ( m_bTethered )
	{
		m_position = homePos;
		m_cameraFrustum.setPosition( m_position );
		m_widget.setPosition( m_position );
	}
	resetToDefault( gph );
}

void Camera::render( const size_t channels ) const
{
	if ( m_bShowWidget )
	{
		m_widget.render( channels );
	}
	if ( m_bShowFrustum )
	{
		m_cameraFrustum.render( channels );
	}
}

void Camera::makeActive( Graphics &gph,
	bool bOrthographic ) const
{
	gph.setViewMatrix( getViewMatrix() );
	gph.setProjectionMatrix( bOrthographic ?
		getOrthographicProjectionMatrix( gph.getClientWidth(), gph.getClientHeight() ) :
		getPerspectiveProjectionMatrix() );
}

DirectX::XMMATRIX Camera::getViewMatrix() const noexcept
{
	const dx::XMVECTOR forwardVector{0.0f, 0.0f, 1.0f, 0.0f};
	const auto lookVector = dx::XMVector3Transform( forwardVector,
		dx::XMMatrixRotationRollPitchYaw( m_pitch, m_yaw, 0.0f ) );
	const auto camPosition = dx::XMLoadFloat3( &m_position );
	const auto camTarget = dx::XMVectorAdd( camPosition,
		lookVector );
	const dx::XMVECTOR upVector{0.0f, 1.0f, 0.0f, 0.0f};
	return dx::XMMatrixLookAtLH( camPosition,
		camTarget,
		upVector );
}

DirectX::XMMATRIX Camera::getReflectionViewMatrix( const dx::XMVECTOR &mirrorPlane ) const noexcept
{
	dx::XMMATRIX R = dx::XMMatrixReflect( mirrorPlane );
	return dx::XMMatrixMultiply( getViewMatrix(),
		R );
}

DirectX::XMMATRIX Camera::getPerspectiveProjectionMatrix() const noexcept
{
	return dx::XMMatrixPerspectiveFovLH( m_fovRadians,
		m_aspectRatio,
		m_nearZ,
		m_farZ );
}

DirectX::XMMATRIX Camera::getOrthographicProjectionMatrix( const unsigned viewWidth,
	const unsigned viewHeight ) const noexcept
{
	return dx::XMMatrixOrthographicLH( (float) viewWidth,
		(float) viewHeight,
		m_nearZ,
		m_farZ );
}

void Camera::displayImguiWidgets( Graphics &gph ) noexcept
{
#ifndef FINAL_RELEASE
	bool projDirty = false;
	bool rotDirty = false;
	bool posDirty = false;
	const auto dirtyCheck = []( bool d, bool &flag )
	{
		flag = flag || d;
	};
	// if the camera is tethered disable its imgui controls
	if ( !m_bTethered )
	{
		ImGui::Text( "Position" );
		dirtyCheck( ImGui::SliderFloat( "X", &m_position.x, -80.0f, 80.0f, "%.1f" ),
			posDirty );
		dirtyCheck( ImGui::SliderFloat( "Y", &m_position.y, -80.0f, 80.0f, "%.1f" ),
			posDirty );
		dirtyCheck( ImGui::SliderFloat( "Z", &m_position.z, -80.0f, 80.0f, "%.1f" ),
			posDirty );
	}
	ImGui::Text( "Orientation" );
	// set m_pitch to 99.5% of +-90 degrees max to prevent euler angle gimbal lock
	dirtyCheck( ImGui::SliderAngle( "Pitch", &m_pitch, 0.995f * -90.0f, 0.995f * 90.0f ),
		rotDirty );
	dirtyCheck( ImGui::SliderAngle( "Yaw", &m_yaw, -180.0f, 180.0f ),
		rotDirty );

	ImGui::Text( "Projection" );
	dirtyCheck( ImGui::SliderFloat( "Width", &m_width, 0.01f, 4.0f, "%.2f", 1.5f ),
		projDirty );
	dirtyCheck( ImGui::SliderFloat( "Height", &m_height, 0.01f, 4.0f, "%.2f", 1.5f ),
		projDirty );
	dirtyCheck( ImGui::SliderFloat( "Near Z", &m_nearZ, 0.01f, m_farZ - 0.01f, "%.2f", 4.0f ),
		projDirty );
	dirtyCheck( ImGui::SliderFloat( "Far Z", &m_farZ, m_nearZ + 0.01f, 1000.0f, "%.2f", 4.0f ),
		projDirty );

	ImGui::Checkbox( "Camera Widget",
		&m_bShowWidget );
	ImGui::Checkbox( "Camera Frustum",
		&m_bShowFrustum );
	if ( ImGui::Button( "Reset" ) )
	{
		resetToDefault( gph );
	}

	if ( rotDirty )
	{
		const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
		m_cameraFrustum.setRotation( angles );
		m_widget.setRotation( angles );
	}
	if ( posDirty )
	{
		m_cameraFrustum.setPosition( m_position );
		m_widget.setPosition( m_position );
	}
	if ( projDirty )
	{
		m_cameraFrustum.setupVertexBuffer( gph,
			m_width,
			m_height,
			m_nearZ,
			m_farZ );
	}
#endif
}

const float Camera::getFovRadians() const noexcept
{
	return m_fovRadians;
}

void Camera::resetToDefault( Graphics &gph ) noexcept
{
	if ( !m_bTethered )
	{
		m_position = m_homePosition;
		m_cameraFrustum.setPosition( m_position );
		m_widget.setPosition( m_position );
	}
	m_pitch = m_homePitch;
	m_yaw = m_homeYaw;

	const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
	m_widget.setRotation( angles );
	m_cameraFrustum.setRotation( angles );
	m_width = m_homeWidth;
	m_height = m_homeHeight;
	m_nearZ = m_homeNearZ;
	m_farZ = m_homeFarZ;
	m_cameraFrustum.setupVertexBuffer( gph,
		m_width,
		m_height,
		m_nearZ,
		m_farZ );
}

void Camera::rotateRel( const float dx,
	const float dy ) noexcept
{
	m_yaw = util::wrapAngle( m_yaw + dx * m_rotationSpeed );
	m_pitch = std::clamp( m_pitch + dy * m_rotationSpeed,
		0.995f * -util::PI / 2.0f,
		0.995f * util::PI / 2.0f );
	const dx::XMFLOAT3 angles{m_pitch, m_yaw, 0.0f};
	m_cameraFrustum.setRotation( angles );
	m_widget.setRotation( angles );
}

void Camera::translateRel( DirectX::XMFLOAT3 translation ) noexcept
{
	if ( !m_bTethered )
	{
		dx::XMStoreFloat3( &translation,
			dx::XMVector3Transform( dx::XMLoadFloat3( &translation ),
				dx::XMMatrixRotationRollPitchYaw( m_pitch, m_yaw, 0.0f ) *
					dx::XMMatrixScaling( m_travelSpeed, m_travelSpeed, m_travelSpeed ) ) );
		m_position = {m_position.x + translation.x,
			m_position.y + translation.y,
			m_position.z + translation.z};
		m_cameraFrustum.setPosition( m_position );
		m_widget.setPosition( m_position );
	}
}

const DirectX::XMFLOAT3& Camera::getPosition() const noexcept
{
	return m_position;
}

DirectX::XMVECTOR Camera::calcDirection() const noexcept
{
	const dx::XMVECTOR forwardVector{0.0f, 0.0f, 1.0f, 0.0f};
	const auto lookVector = dx::XMVector3Transform( forwardVector,
		dx::XMMatrixRotationRollPitchYaw( m_pitch, m_yaw, 0.0f ) );
	const auto camPosition = dx::XMLoadFloat3( &m_position );
	const auto camTarget = dx::XMVectorAdd( camPosition,
		lookVector );
	return dx::XMVector3Normalize( dx::XMVectorSubtract( camPosition,
		camTarget ) );
}

DirectX::XMVECTOR Camera::calcRight() const noexcept
{
	const dx::XMVECTOR upVector{0.0f, 1.0f, 0.0f, 0.0f};
	return dx::XMVector3Cross( upVector,
		calcDirection() );
}

DirectX::XMVECTOR Camera::calcUp() const noexcept
{
	return dx::XMVector3Cross( calcDirection(),
		calcRight() );
}

void Camera::setPosition( const DirectX::XMFLOAT3 &pos ) noexcept
{
	this->m_position = pos;
	m_cameraFrustum.setPosition( pos );
	m_widget.setPosition( pos );
}

const std::string& Camera::getName() const noexcept
{
	return m_name;
}

void Camera::connectEffectsToRenderer( ren::Renderer &ren )
{
	m_cameraFrustum.connectEffectsToRenderer( ren );
	m_widget.connectEffectsToRenderer( ren );
}

std::vector<dx::XMFLOAT4> Camera::getFrustumPlanes() const noexcept
{
	// x, y, z, and w represent A, B, C and D in the plane equation
	// where ABC are the xyz of the planes normal, and D is the plane constant
	std::vector<dx::XMFLOAT4> frustumPlanes( 6 );

	dx::XMFLOAT4X4 viewProj;	// Left-Handed
	dx::XMStoreFloat4x4( &viewProj,
		getViewMatrix() * getPerspectiveProjectionMatrix() );

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