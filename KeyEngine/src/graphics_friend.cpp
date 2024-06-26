#include "graphics_friend.h"
#include "graphics.h"


ID3D11DeviceContext* GraphicsFriend::getDeviceContext( Graphics &gfx ) noexcept
{
	return gfx.m_pImmediateContext.Get();
}

ID3D11Device* GraphicsFriend::getDevice( Graphics &gfx ) noexcept
{
	return gfx.m_pDevice.Get();
}