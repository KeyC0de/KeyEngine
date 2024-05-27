#include "settings_manager.h"
#include "winner.h"
#include "console.h"
#include "utils.h"
#include "math_utils.h"
#include "inih/INIReader.h"


SettingsManager::SettingsManager( const std::string &filePath )
{
	loadFromFile( filePath );
}

SettingsManager& SettingsManager::getInstance( const std::string &filePath )
{
	static SettingsManager s_instance{std::string{"config/"} + filePath};
	return s_instance;
}

void SettingsManager::loadFromFile( const std::string &filePath )
{
	INIReader ini{filePath};

	if ( ini.ParseError() < 0 )
	{
		std::wstring msgText = L"Couldn't not load file " + util::s2ws( filePath );
		MessageBoxW( nullptr, msgText.data(), L"File not found!", MB_OK | MB_ICONEXCLAMATION );
		std::terminate();
	}

	m_settings.version = ini.GetInteger( "System", "Version", -1 );
	m_settings.nThreads = (unsigned) ini.GetInteger( "System", "nThreads", 4 );

	if ( ini.GetInteger( "GameSelection", "Sandbox3d", 0 ) )
	{
		m_settings.m_game = "Sandbox3d";
	}
	else if ( ini.GetInteger( "GameSelection", "Arkanoid2d", 0 ) )
	{
		m_settings.m_game = "Arkanoid2d";
	}
	else if( ini.GetInteger( "GameSelection", "Minesweeper2d", 0 ) )
	{
		m_settings.m_game = "Minesweeper2d";
	}
	else
	{
		MessageBoxW( nullptr, L"No game selected in the config.ini file. Please specify a game.", L"No Selected Game!", MB_OK | MB_ICONEXCLAMATION );
		std::terminate();
	}

	m_settings.fGameSpeed = static_cast<float>( ini.GetReal( "Gameplay", "fGameSpeed", 1.0f ) );

	m_settings.bVSync = ini.GetBoolean( "Graphics", "bVSync", true );
	m_settings.iMaxFps = ini.GetInteger( "Graphics", "iMaxFps", -1 );
	m_settings.bAllowWindowResize = ini.GetBoolean( "Graphics", "bAllowWindowResize", false );
	m_settings.bEnableFrustumCuling = ini.GetBoolean( "Graphics", "bEnableFrustumCuling", true );
	m_settings.iPresentInterval = util::clamp( ini.GetInteger( "Graphics", "iPresentInterval", 1 ), 0l, 4l );
	
	m_settings.sSkyboxFileName = ini.Get( "Assets", "sSkyboxFileName", "" );

	m_settings.sFontName = ini.Get( "Graphics", "sFontName", "myComicSansMSSpriteFont" );
}

SettingsManager::Settings& SettingsManager::getSettings()
{
	return m_settings;
}

const SettingsManager::Settings& SettingsManager::getSettings() const
{
	return m_settings;
}