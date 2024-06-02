#pragma once

#include <d3d11.h>


class Graphics;

class GraphicsFriend
{
protected:
	static ID3D11DeviceContext* getDeviceContext( Graphics &gfx ) noexcept;
	static ID3D11Device* getDevice( Graphics &gfx ) noexcept;
};