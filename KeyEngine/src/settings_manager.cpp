#include "settings_manager.h"
#include "winner.h"
#include "INIReader.h"
#include "console.h"
#include "utils.h"
#include "console.h"


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

	m_settings.version = ini.GetInteger( "Protocol", "Version", -1 );

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

#if defined _DEBUG && !defined NDEBUG
	KeyConsole& console = KeyConsole::getInstance();
	console.log( "\n\nConfig loaded from \"" + filePath + "\" - Provided patemeters: {"
		"\n\tversion="
		+ std::to_string( m_settings.version )
		+ "\n\tgame="
		+ std::string{m_settings.m_game}
		//+ "\n\tpi="
		//+ ini.GetReal( "user", "pi", -1 )
		//+ "\n\tactive="
		//+ ini.GetBoolean( "user", "active", true )
		+ "\n}\n\n" );
#endif
}

SettingsManager::Settings& SettingsManager::getSettings()
{
	return m_settings;
}

const SettingsManager::Settings& SettingsManager::getSettings() const
{
	return m_settings;
}