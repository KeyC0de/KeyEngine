#include "imgui_manager.h"
#ifndef FINAL_RELEASE
#	include <filesystem>
#	include "imgui.h"


ImguiManager::ImguiManager()
{
	namespace fs = std::filesystem;
	if ( !fs::exists( "config/imgui.ini" ) && fs::exists( "config/imgui_default.ini" ) )
	{
		fs::copy_file( "config/imgui_default.ini", "config/imgui.ini" );
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

ImguiManager::~ImguiManager()
{
	ImGui::DestroyContext();
}

#endif