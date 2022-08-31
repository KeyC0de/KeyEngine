#include "pixel_shader_null.h"
#include "bindable_map.h"
#include "dxgi_info_queue.h"


PixelShaderNull::PixelShaderNull( Graphics &gph )
{

}

void PixelShaderNull::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->PSSetShader( nullptr,
		nullptr,
		0u );
	DXGI_GET_QUEUE_INFO( gph );
}

std::shared_ptr<PixelShaderNull> PixelShaderNull::fetch( Graphics &gph )
{
	return BindableMap::fetch<PixelShaderNull>( gph );
}

std::string PixelShaderNull::calcUid()
{
	return typeid( PixelShaderNull ).name();
}

const std::string PixelShaderNull::getUid() const noexcept
{
	return calcUid();
}