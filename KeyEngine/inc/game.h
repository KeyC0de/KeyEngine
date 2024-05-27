#pragma once

#include <optional>
#include <memory>
#include <type_traits>
#include "window.h"
#include "reporter_listener.h"
#include "reporter_listener_events.h"
#include "key_timer.h"
#include "camera_manager.h"
#include "light_source.h"
#include "terrain.h"
#include "plane.h"
#include "line.h"
#include "cube.h"
#include "sphere.h"
#include "model.h"
#include "non_copyable.h"
#include "settings_manager.h"
#include "game_state.h"
#include "key_sound.h"
#include "rectangle.h"
#include "imgui_manager.h"
#include "arkanoid/ball.h"
#include "arkanoid/brick.h"
#include "arkanoid/paddle.h"
#include "math_utils.h"


namespace ren
{
	class Renderer;
}

namespace gui
{
	class UIPass;
}

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

	//	\function	present	||	\date	2022/11/05 16:33
	//	\brief	all child Games must run this function as the very last statement in their render function
	void present();
	std::optional<Window*> getForegroundWindow() const noexcept;
	void setState( std::unique_ptr<State> pNewState, Mouse &mouse );
	const State* getState() const noexcept;
	State* getState() noexcept;
private:
	ImguiManager* createImgui() noexcept;
};

class Sandbox3d
	: public Game<Sandbox3d>,
	public IListener<SwapChainResizedEvent>
{
	static inline CameraManager &s_cameraMan = CameraManager::getInstance();

	std::unique_ptr<gui::UIPass> m_gui;
	std::unique_ptr<PointLight> m_pPointLight1;
	//std::unique_ptr<PointLight> m_pPointLight2;
#ifndef FINAL_RELEASE
	bool m_bShowDemoWindow = false;
#endif
	Terrain m_terrain{m_mainWindow.getGraphics(), 100, 100, "assets/textures/clouds_blurred.bmp", 1.0f, {util::toRadians( 90.0f ), 0.0f, 0.0f}, {0.0f, -100.0f, 0.0f}/*, {0.0f, 1.0f, 0.0f, 1.0f}*/};
	Line m_debugLine1{m_mainWindow.getGraphics(), 4.0f, {0, 0, 0}, {10.0f, 10.0f, 6.0f}, {1.0f, 0.0f, 0.0f, 1.0f}};
	Cube m_cube1{m_mainWindow.getGraphics(), 4.0f, {0, 0, 0}, {10.0f, 5.0f, 6.0f}};
	Cube m_cube2{m_mainWindow.getGraphics(), 1.0f, {0, 0, 0}, {9.9f, 4.9f, 1.4f}, {1.0f, 0.4f, 0.4f, 0.5f}};
	Cube m_cube3{m_mainWindow.getGraphics(), 1.0f, {0, 0, 0}, {22.0f, 12.0f, 14.0f}};
	Sphere m_testSphere{m_mainWindow.getGraphics(), 1.0f, {0, 0, 0}, {40.0f, 20.0f, 8.0f}};
	Plane m_plane1Red{m_mainWindow.getGraphics(), 8, 8, 1.0f, {0, 0, 0}, {40.0f, 10.0f, 20.0f}, {1.0f, 0.0f, 0.1f, 0.8f}, ""};
	Plane m_plane2Green{m_mainWindow.getGraphics(), 4, 4, 1.0f, {0, 0, 0}, {40.0f, 10.0f, 16.0f}, {0.1f, 1.0f, 0.0f, 0.5f}, ""};
	Plane m_plane3Textured{m_mainWindow.getGraphics(), 6, 6, 1.0f, {0, 0, 0}, {40.0f, 10.0f, 12.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};
	//Model m_sponzaScene{m_mainWindow.getGraphics(), "assets/models/sponza/sponza.obj", 1.0f / 8.0f, {0, 0, 0}, {0.0f, 0.0f, 0.0f}};
	Model m_carabiner{m_mainWindow.getGraphics(), "assets/models/carabiner/carabiner_hook.fbx", 1.0f, {0, 0, 0}, {-10.0f, 6.0f, 0.0f}};
public:
	Sandbox3d( const int width, const int height, const int x, const int y, const int nWindows = 1 );
	~Sandbox3d() noexcept;

	void notify( const SwapChainResizedEvent &event ) override;
	int loop();
private:
	int checkInput( const float dt );
	void update( const float dt, const float lerpBetweenFrames );
	void updateFixed( const float dt );
	void render();
	void test();
	void connectToRenderer( ren::Renderer3d &renderer );
};

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
	void update( const float dt );
	void render();
	void test();
};


#define throwGameException( msg ) throw GameException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );