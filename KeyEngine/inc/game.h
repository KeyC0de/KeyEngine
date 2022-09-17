#pragma once

#include <optional>
#include <memory>
#include <type_traits>
#include "window.h"
#include "key_timer.h"
#include "camera_manager.h"
#include "light.h"
#include "cube.h"
#include "sphere.h"
#include "model.h"
#include "non_copyable.h"
#include "renderer.h"
#include "settings_manager.h"
#include "game_state.h"
#include "sound_manager.h"
#include "rectangle.h"
#include "imgui_manager.h"
#include "../games/arkanoid/ball.h"
#include "../games/arkanoid/brick.h"
#include "../games/arkanoid/paddle.h"


template<typename T>
class Game
	: public NonCopyableAndNonMovable
{
	class GameException final
		: public KeyException
	{
	public:
		GameException( const int line, const char *file, const char *function, const std::string &msg ) noexcept;

		const std::string getType() const noexcept override final;
		virtual const char* what() const noexcept override final;
	};
protected:
	static inline unsigned m_nWindows;
	static inline SettingsManager &m_settingsMan = SettingsManager::instance();
	std::unique_ptr<ImguiManager> m_pImguiMan;		// deleted 2nd
	Window m_mainWindow;							// deleted 1st
	std::unique_ptr<State> m_pCurrentState;
	KeyTimer<std::chrono::milliseconds> m_gameTimer;
public:
	~Game() noexcept = default;

protected:
	Game( const int width, const int height, const std::string &title, const unsigned nWindows = 1 );

	const float calcDt();
	std::optional<Window*> getForegroundWindow() const noexcept;
	void setState( std::unique_ptr<State> pNewState, Mouse &mouse );
	const State* getState() const noexcept;
	State& state() noexcept;
private:
	ImguiManager* createImgui() noexcept;
};

class Sandbox3d
	: public Game<Sandbox3d>
{
	static inline CameraManager& m_cameraMan = CameraManager::instance();
	ren::Renderer3d m_renderer;
	std::unique_ptr<PointLight> m_pPointLight1;
	//std::unique_ptr<PointLight> m_pPointLight2;
	bool b_bShowDemoWindow = false;
	Cube m_cube1{m_mainWindow.getGraphics(), {10.0f, 5.0f, 6.0f}, 4.0f};
	Cube m_cube2{m_mainWindow.getGraphics(), {16.0f, 6.0f, 8.0f}};
	Sphere m_testSphere{m_mainWindow.getGraphics(), 1.0f, {40.0f, 20.0f, 8.0f}};
	Model m_sponzaScene{m_mainWindow.getGraphics(), "assets/models/sponza/sponza.obj", 1.0f / 8.0f};
	Model m_nanoSuit{m_mainWindow.getGraphics(), "assets/models/nano_textured/nanosuit.obj", 2.0f};
	Model m_carabiner{m_mainWindow.getGraphics(), "assets/models/carabiner/carabiner_hook.fbx", 1.0f};
public:
	Sandbox3d( const int width, const int height, const int nWindows = 1 );

	int loop();
private:
	int checkInput( const float dt );
	void update( const float dt );
#if defined _DEBUG && !defined NDEBUG
	void test();
#endif
	void render( const float dt );
	void present();
	void renderImgui();
};

class Arkanoid final
	: public Game<Arkanoid>
{
	static constexpr inline float m_brickWidth = 40.0f;
	static constexpr inline float m_brickHeight = 20.0f;
	static constexpr inline int m_nBricksHorizontally = 18;
	static constexpr inline int m_nBricksVertically = 4;
	static constexpr inline int m_nBricks = m_nBricksHorizontally * m_nBricksVertically;
	static constexpr inline float m_speed = 300.0f;
	ren::Renderer2d m_renderer;
	Ball m_ball;
	Rect m_walls;
	Brick m_bricks[m_nBricks];
	Paddle m_paddle;
	Sound m_brickSound;
	Sound m_padSound;
public:
	Arkanoid( const int width, const int height );

	int loop();
private:
	int checkInput( const float dt );
	void update( const float dt );
#if defined _DEBUG && !defined NDEBUG
	void test();
#endif
	void render( const float dt );
	void present();
};


#define throwGameException( msg ) throw GameException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );