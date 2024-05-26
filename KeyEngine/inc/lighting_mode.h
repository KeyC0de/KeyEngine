#pragma once


namespace lgh_mode
{

enum LightingMode
{
	BlinnPhong,
	PBR_UE,
};

static constexpr LightingMode s_currentLightingModel = BlinnPhong;	/// you have to manually change this to switch modes (and potentially games)

static inline constexpr const int get() noexcept
{
	return s_currentLightingModel;
}


}//namespace lgh_mode