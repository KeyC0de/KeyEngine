#include "graphics_friend.h"


ID3D11DeviceContext* GraphicsFriend::getDeviceContext( Graphics &gph ) noexcept
{
	return gph.m_pImmediateContext.Get();
}

ID3D11Device* GraphicsFriend::getDevice( Graphics &gph ) noexcept
{
	return gph.m_pDevice.Get();
}