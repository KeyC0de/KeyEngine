#include "graphics_friend.h"


ID3D11DeviceContext* GraphicsFriend::getContext( Graphics& gph ) noexcept
{
	return gph.m_pContext.Get();
}

ID3D11Device* GraphicsFriend::getDevice( Graphics& gph ) noexcept
{
	return gph.m_pDevice.Get();
}