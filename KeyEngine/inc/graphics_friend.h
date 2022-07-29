#pragma once

#include "graphics.h"


class GraphicsFriend
{
protected:
	static ID3D11DeviceContext *getContext( Graphics &gph ) noexcept;
	static ID3D11Device *getDevice( Graphics &gph ) noexcept;
};