#include "settings_manager.h"
#include "winner.h"
#include "INIReader.h"
#include "console.h"
#include "utils.h"


SettingsManager::SettingsManager( const std::string &filePath )
{
	//loadFromFile( filePath );
}

SettingsManager::~SettingsManager() noexcept
{
	// free allocated resources if any
		
}

SettingsManager &SettingsManager::getInstance( const std::string &filePath )
{
	if ( m_pInstance == nullptr )
	{
		m_pInstance = new SettingsManager{filePath};
	}
	return *m_pInstance;
}

void SettingsManager::resetInstance()
{
	if ( m_pInstance != nullptr )
	{
		delete m_pInstance;
	}
}

void SettingsManager::loadFromFile( const std::string &filePath )
{
	INIReader ini( filePath );

	if ( ini.ParseError() < 0 )
	{
		std::wstring msgText = L"Couldn't not load file "
			+ util::s2ws( filePath );
		MessageBoxW( nullptr,
			msgText.data(),
			L"File not found!",
			MB_OK | MB_ICONEXCLAMATION );
		std::terminate();
	}
	// convert inih string inputs/outputs to wstring
	//console.log( "Config loaded from " + filePath + ":\n"
	//	"version="
	//	+ ini.GetInteger( "protocol", "version", -1 )
	//	<< L", name="
	//	<< s2ws( ini.Get( "user", "name", "UNKNOWN" ) )
	//	<< L", greekName="
	//	<< s2ws( ini.Get( "user", "greekName", "UNKNOWN" ) )
	//	<< L", email="
	//	<< s2ws( ini.Get( "user", "email", "UNKNOWN" ) )
	//	<< L", pi="
	//	<< ini.GetReal( "user", "pi", -1 )
	//	<< L", active=" 
	//	<< ini.GetBoolean("user", "active", true) << "\n";
}

const SettingsManager::Settings &SettingsManager::getSettings()
{
	return m_settings;
}

SettingsManager::Settings &SettingsManager::accessSettings()
{
	return m_settings;
}