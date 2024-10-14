#pragma once

#include <string>
#include <string_view>
#include "non_copyable.h"


//=============================================================
//	\class	SettingsManager
//	\author	KeyC0de
//	\date	2022/05/03 12:07
//	\brief	singleton class
//=============================================================
class SettingsManager final
	: public NonCopyable
{
	struct Settings
	{
		// defaults:
		int version = 0;
		unsigned nThreads = 4;
		std::string_view m_game = "Sandbox3d";
		float fGameSpeed = 1.0f;
		bool bVSync = true;
		bool bFpsCounting = true;
		int frameCount = 0;
		int iMaxFps = -1;
		int iMaxShadowCastingDynamicLights = 8;
		int iMaxShadowCastingPointLights = 6;
		unsigned iPresentInterval = 1u;
		bool bStaticShaderCompilation = true;
		bool bMultithreadedRendering = false;
		unsigned nRenderingThreads = 4u;
		bool bGamePaused = false;
		bool bFullscreen = false;
		bool bAllowWindowResize = false;
		bool bEnableFrustumCuling = true;
		bool bEnableSmoothMovement = true;
		std::string sSkyboxFileName = "";
		std::string sFontName = "myComicSansMSSpriteFont";
	} m_settings;
private:
	SettingsManager( const std::string &filePath );

	void loadFromFile( const std::string &filePath );
public:
	static SettingsManager& getInstance( const std::string &filePath = "config.ini" );
	Settings& getSettings();
	const Settings& getSettings() const;
};