#pragma once


struct ImguiManager final
{
	ImguiManager();
	~ImguiManager();
	ImguiManager( const ImguiManager &rhs ) = delete;
	ImguiManager &operator=( const ImguiManager &rhs ) = delete;

	static ImguiManager *getInstance();
};