#pragma once

//
//										   _-- Bindable1
//					  _-- Pass1|Effect1 --+--- Bindable2
// RenderingChannel -+--- Pass2|Effect2	   --- BindableN
//					  --- PassN|EffectN
//

// #TODO: turn it into an enum class
// #TODO: turn their names into the Passes names using enum and fetch their names from here
namespace rch
{

static constexpr size_t lambert = 0b1;
static constexpr size_t shadow = 0b10;
static constexpr size_t solidOutline = 0b100;
static constexpr size_t blurOutline = 0b1000;
static constexpr size_t wireframe = 0b10000;

static constexpr size_t all = lambert | shadow | solidOutline | blurOutline | wireframe;

}