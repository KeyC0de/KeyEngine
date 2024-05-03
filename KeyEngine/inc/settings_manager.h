#pragma once

#include <string>
#include <string_view>


//=============================================================
//	\class	SettingsManager
//	\author	KeyC0de
//	\date	2022/05/03 12:07
//	\brief	singleton class
//=============================================================
class SettingsManager final
{
	static inline SettingsManager *s_pInstance;
	struct Settings
	{
		// defaults:
		int version = 0;
		std::string_view m_game = "Sandbox3d";
		float fGameSpeed = 1.0f;
		bool bVSync = true;
		bool bFpsCounting = true;
		int frameCount = 0;
		int iMaxFps = -1;
		unsigned iPresentInterval = 1u;
		bool bStaticShaderCompilation = true;
		bool bMultithreadedRendering = false;
		unsigned nRenderingThreads = 4u;
		bool bGamePaused = false;
		bool bFullscreen = false;
		bool bAllowWindowResize = false;
	} m_settings;
private:
	SettingsManager( const std::string &filePath );

	void loadFromFile( const std::string &filePath );
public:
	static SettingsManager& getInstance( const std::string &filePath = "config.ini" );
	Settings& getSettings();
	const Settings& getSettings() const;
};