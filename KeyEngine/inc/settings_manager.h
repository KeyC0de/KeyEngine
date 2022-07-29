#pragma once

#include <string>


class SettingsManager final
{
	static inline SettingsManager *m_pInstance;
	struct Settings
	{
		// defaults:
		float fGameSpeed = 1.0f;
		bool bVSync = true;
		bool bFpsCap = false;
		bool bFpsCounting = true;
		int frameCount = 0;
		int iMaxFps = 60;
		bool bStaticShaderCompilation = true;
		bool bMultithreadedRendering = false;
		unsigned nRenderingThreads = 4u;
		bool bGamePaused = false;
		bool bFullscreen = false;
	} m_settings;

	SettingsManager( const std::string &filePath );

	void loadFromFile( const std::string &filePath );
public:
	~SettingsManager() noexcept;
	SettingsManager( const SettingsManager &rhs ) = delete;
	SettingsManager& operator=( const SettingsManager &rhs ) = delete;

	static SettingsManager &getInstance( const std::string &filePath = "config.ini" );
	static void resetInstance();
	const Settings& getSettings();
	Settings& accessSettings();
};