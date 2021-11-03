#include <filesystem>
#include "imgui_manager.h"
#include "imgui.h"


ImguiManager::ImguiManager()
{
	namespace fs = std::filesystem;
	if ( !fs::exists( "imgui.ini" ) && fs::exists( "imgui_default.ini" ) )
	{
		fs::copy_file( "imgui_default.ini",
			"imgui.ini" );
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

ImguiManager::~ImguiManager()
{
	ImGui::DestroyContext();
}

ImguiManager* ImguiManager::getInstance()
{
	static ImguiManager instance{};
	return &instance;
}