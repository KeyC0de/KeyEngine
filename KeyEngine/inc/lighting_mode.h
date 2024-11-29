#pragma once


namespace lgh_mode
{

enum LightingMode
{
	BlinnPhong,
	PBR_UE,
};

/// \brief you have to manually change this to switch modes (and potentially games)
static constexpr LightingMode s_currentLightingModel = BlinnPhong;

static inline constexpr const int get() noexcept
{
	return s_currentLightingModel;
}


}//namespace lgh_mode