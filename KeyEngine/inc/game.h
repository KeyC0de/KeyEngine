#pragma once

#include <optional>
#include <memory>
#include <type_traits>
#include "window.h"
#include "key_timer.h"
#include "camera_manager.h"
#include "light.h"
#include "cube.h"
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
//#include "../games/snake/snake_play_field.h"
//#include "../games/snake/snake.h"
//#include "../games/snake/fruit.h"


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
	static inline SettingsManager &m_settingsMan = SettingsManager::getInstance();
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
	static inline CameraManager& m_cameraMan = CameraManager::getInstance();
	ren::Renderer3d m_renderer;
	std::unique_ptr<PointLight> m_pPointLight1;
	//std::unique_ptr<PointLight> m_pPointLight2;
	bool b_bShowDemoWindow = false;
	Cube m_cube1{m_mainWindow.getGraphics(), {10.0f, 5.0f, 6.0f}, 4.0f};
	Cube m_cube2{m_mainWindow.getGraphics(), {16.0f, 6.0f, 8.0f}};
	Model m_sponzaScene{m_mainWindow.getGraphics(), "assets/models/sponza/sponza.obj", 1.0f/8.0f};
	Model m_nanoSuit{m_mainWindow.getGraphics(), "assets/models/nano_textured/nanosuit.obj", 2.0f};
	Model m_carabiner{m_mainWindow.getGraphics(), "assets/models/carabiner/carabiner_hook.fbx", 1.0f};
public:
	Sandbox3d( int const width, const int height, const int nWindows = 1 );

	int loop();
private:
	void checkInput( const float dt );
	void update( const float dt );
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
	void checkInput( const float dt );
	void update( const float dt );
	void render( const float dt );
	void present();
};

/*
class Snake final
	: public Game<Snake>
{
	std::mt19937 rng;
	Board board;
	Snake snek;
	Location2d delta_loc = { 1, 0 };	// 2d orientation and displacement
	Fruit fruit;
	float snakeMovePeriod = 0.4f;
	float snakeMoveCounter = 0.0f;
	float m_snekModifiedMovePeriod;
	float snekSpeedupFactor = 0.97;
	int fruitsEaten = 0;
	std::normal_distribution<float> freqDist;
	int nPoison;
	static constexpr float snekMovePeriodMin = 0.040f;
	static constexpr float howManyFruitsForSpeedUp = 0.15f;
	int gameIsOver = false;
	int nHowManyFruits = 2;
	Sound sfxEat = Sound{L"Sounds\\Eat.wav"};
	ren::Renderer2d m_renderer;
public:
	Snake( const int width, const int height );

	int loop();
private:
	void checkInput( const float dt );
	void update( const float dt );
	void render( const float dt );
	void present();
};*/


#define throwGameException( msg ) throw GameException( __LINE__,\
	__FILE__,\
	__FUNCTION__,\
	msg );