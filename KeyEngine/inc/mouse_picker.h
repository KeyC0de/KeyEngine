#pragma once

#include <DirectXMath.h>


class Graphics;

// #TODO:
class MousePicker
{
	DirectX::XMFLOAT3 m_rayDirectionWorldSpace;
public:
	MousePicker( Graphics &gph, int screenX, int screenY );

	DirectX::XMFLOAT2 convertToNdc( Graphics &gph, const DirectX::XMFLOAT2 &coordsScreenSpace );
	DirectX::XMFLOAT2 convertToClip( Graphics &gph, const DirectX::XMFLOAT2 &coordsNdc );
	DirectX::XMVECTOR convertToViewSpace( Graphics &gph, const DirectX::XMFLOAT2 &coordsClip );
	DirectX::XMVECTOR convertToWorldSpace( Graphics &gph, const DirectX::XMFLOAT4 &coordsView );
};