#pragma once

/// \brief	
/// \brief												--- Bindable1
/// \brief					  _-- Pass1|Material1 ----+---- Bindable2
/// \brief	RenderingChannel -+--- Pass2|Material2		--- BindableN
/// \brief					  --- PassN|MaterialN
/// \brief	

// #TODO: turn it into an enum class
// #TODO: turn their names into the Passes names using enum and fetch their names from here
namespace rch
{

static constexpr size_t opaque = 0b1;
static constexpr size_t shadow = 0b10;
static constexpr size_t solidOutline = 0b100;
static constexpr size_t blurOutline = 0b1000;
static constexpr size_t wireframe = 0b10000;
static constexpr size_t transparent = 0b100000;

static constexpr size_t all_opaque = opaque | shadow | solidOutline | blurOutline | wireframe;
static constexpr size_t all = all_opaque | transparent;


}//namespace rch