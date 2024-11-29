#pragma once

#include <optional>
#include <memory>
#include <vector>
#include <type_traits>
#include "window.h"
#include "reporter_listener.h"
#include "reporter_listener_events.h"
#include "key_timer.h"
#include "camera_manager.h"
#include "light_source.h"
#include "terrain.h"
#include "model.h"
#include "non_copyable.h"
#include "settings_manager.h"
#include "key_sound.h"
#include "rectangle.h"
#include "arkanoid/ball.h"
#include "arkanoid/brick.h"
#include "arkanoid/paddle.h"
#ifndef FINAL_RELEASE
#	include "imgui_manager.h"
#endif


namespace ren
{
class Renderer;
class Renderer3d;
}

namespace gui
{
class UIPass;
}

class State;

///=============================================================
/// \class	Game
/// \author	KeyC0de
/// \date	2020/01/21 14:25
/// \brief	contains the main game loop, the Window and the #TODO: Worlds - each World is a graphical scene/map to be loaded at a time
///=============================================================
template<typename T>
class Game
	: public NonCopyableAndNonMovable
{
	class GameException final
		: public KeyException
	{
	public:
		GameException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};
protected:
	static inline unsigned s_nWindows;
	static inline SettingsManager &s_settingsMan = SettingsManager::getInstance();
#ifndef FINAL_RELEASE
	std::unique_ptr<ImguiManager> m_pImguiMan;		// deleted 2nd
#endif
	Window m_mainWindow;							// deleted 1st
	std::unique_ptr<State> m_pCurrentState;
	KeyTimer<std::chrono::milliseconds> m_gameTimer;
public:
	~Game() noexcept = default;

protected:
	Game( const int width, const int height, const std::string &title, const int x, const int y, const unsigned nWindows = 1 );

	/// \brief	all child Games must run this function as the very last statement in their render function
	void present( Graphics &gfx );
	std::optional<Window*> getForegroundWindow() const noexcept;
	void setState( std::unique_ptr<State> pNewState, Mouse &mouse );
	const State* getState() const noexcept;
	State* getState() noexcept;
private:
#ifndef FINAL_RELEASE
	ImguiManager* createImguiManager() noexcept;
#endif
};

class Sandbox3d
	: public Game<Sandbox3d>,
	public IListener<SwapChainResizedEvent>
{
	static inline CameraManager &s_cameraMan = CameraManager::getInstance();

	std::unique_ptr<gui::UIPass> m_gui;
#ifndef FINAL_RELEASE
	bool m_bShowDemoWindow = false;
#endif
	std::vector<std::unique_ptr<ILightSource>> m_lights;
	Model m_terrain{std::make_unique<Terrain>(m_mainWindow.getGraphics(), 1.0f, DirectX::XMFLOAT4{0.1f, 0.8f, 0.05f, 1.0f}, "assets/textures/clouds_blurred.bmp", 100, 100), m_mainWindow.getGraphics(), {90.0f, 0.0f, 0.0f}, {0.0f, -100.0f, 0.0f}};
	std::vector<Model> m_models;
public:
	Sandbox3d( const int width, const int height, const int x, const int y, const int nWindows = 1 );
	~Sandbox3d() noexcept;

	void notify( const SwapChainResizedEvent &event ) override;
	int loop();
private:
	int checkInput( const float dt );
	void update( Graphics &gfx, const float dt, const float lerpBetweenFrames );
	void updateFixed( const float dt );
	void render( Graphics &gfx );
	void test( Graphics &gfx );
	void connectToRenderer( ren::Renderer3d &renderer );
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class Arkanoid final
	: public Game<Arkanoid>
{
	static constexpr inline float s_brickWidth = 40.0f;
	static constexpr inline float s_brickHeight = 20.0f;
	static constexpr inline int s_nBricksHorizontally = 18;
	static constexpr inline int s_nBricksVertically = 4;
	static constexpr inline int s_nBricks = s_nBricksHorizontally * s_nBricksVertically;
	static constexpr inline float s_speed = 300.0f;
	Ball m_ball;
	RectangleF m_walls;
	Brick m_bricks[s_nBricks];
	Paddle m_paddle;
	Sound m_brickSound;
	Sound m_padSound;
public:
	Arkanoid( const int width, const int height, const int x, const int y );

	int loop();
private:
	float calcDt();
	int checkInput( const float dt );
	void update( Graphics &gfx, const float dt );
	void render( Graphics &gfx );
	void test();
};


#define throwGameException( msg ) throw GameException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );