#include "game.h"
#include <algorithm>
#ifndef FINAL_RELEASE
#	include "imgui.h"
#endif
#include "imgui_visitors.h"
#include "camera.h"
#include "rendering_channel.h"
#include "console.h"
#include "assertions_console.h"
#include "mesh.h"
#include "graphics_mode.h"
#include "jthread_pool.h"
#include "utils.h"
#include "os_utils.h"
#include "color.h"
#include "..\resource.h"
#include "renderer.h"
#if defined _DEBUG && !defined NDEBUG
#	include "bindable_map.h"
#endif


namespace dx = DirectX;

template <typename T>
Game<T>::Game( const int width,
	const int height,
	const std::string &title,
	const int x,
	const int y,
	const unsigned nWindows )
	:
#ifndef FINAL_RELEASE
	m_pImguiMan{createImgui()},
#endif
	m_mainWindow{width, height, title.c_str(), MAIN_WINDOW_CLASS_NAME, Window::windowProc, x, y, ColorBGRA{255, 255, 255}, LoadMenuW( THIS_INSTANCE, MAKEINTRESOURCEW( IDR_MENU_APP ) )},
	m_pCurrentState{std::make_unique<GameState>()}
{
	s_nWindows = nWindows;
}

template <typename T>
ImguiManager* Game<T>::createImgui() noexcept
{
#ifndef FINAL_RELEASE
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		return new ImguiManager{};
	}
	else
	{
		return nullptr;
	}
#else
	return nullptr;
#endif
}

template <typename T>
std::optional<Window*> Game<T>::getForegroundWindow() const noexcept
{
	static const INT_PTR mainWindowHandle = (INT_PTR) m_mainWindow.getHandle();
	//static const INT_PTR consoleWindowHandle = (INT_PTR) m_consoleWindow.getHandle();
	//static const INT_PTR helperWindowHandle = (INT_PTR) m_helperWindow.getHandle();
	INT_PTR pWnd = (INT_PTR) GetForegroundWindow();

	if ( pWnd == mainWindowHandle )
	{
		return &m_mainWindow;
	}

	return std::nullopt;
}

template <typename T>
void Game<T>::setState( std::unique_ptr<State> pNewState,
	Mouse &mouse )
{
	ASSERT( pNewState, "New state is null!" );
	if ( dynamic_cast<GameState*>( pNewState.get() ) )
	{
		m_mainWindow.disableCursor();
		mouse.enableRawInput();
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		console.print( "\nGame state is on.\n" );
#endif
	}
	else if ( dynamic_cast<MenuState*>( pNewState.get() ) != nullptr )
	{
		m_mainWindow.enableCursor();
		mouse.disableRawInput();
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		console.print( "\nMenu state is on.\n" );
#endif
	}
	else
	{
		throwGameException( "Invalid state!" );
	}
	m_pCurrentState.swap( pNewState );
}

template <typename T>
const State* Game<T>::getState() const noexcept
{
	return m_pCurrentState.get();
}

template <typename T>
State* Game<T>::getState() noexcept
{
	return m_pCurrentState.get();
}

template <typename T>
float Game<T>::calcDt()
{
	auto &settings = s_settingsMan.getSettings();
	float dt = m_gameTimer.lap() * settings.fGameSpeed;

#if defined _DEBUG && !defined NDEBUG
	// print frame time
	KeyConsole &console = KeyConsole::getInstance();
	using namespace std::string_literals;
	++settings.frameCount;
	std::string frameStats = "Frame time : "s
		+ std::to_string( dt )
		+ "ms. Frame "s
		+ std::to_string( settings.frameCount )
		+ "\n"s;
	console.print( frameStats );
#endif // _DEBUG
//	static float minFrameTime = 1.0f / settings.iMaxFps;
//	if ( settings.bFpsCap && !( settings.bFullscreen == true && settings.bVSync == true ) )
//	{
//		// enable frame limiter
//		// Put on a low bound on the timestep - any lower and it would exceed maxFps
//		//dt /= 1000;
//		const float remainingTime = dt - minFrameTime;
//		if ( remainingTime < 0.0f )
//		{
//			m_gameTimer.delayFor( remainingTime );
//			//dt = std::min<float>( dt, m_minTimeStep );
//			dt = minFrameTime;
//		}
//	}
	return dt;
}

template<typename T>
void Game<T>::present()
{
	auto &gph = m_mainWindow.getGraphics();
	gph.endFrame();
}

template<typename T>
Game<T>::GameException::GameException( const int line,
	const char *file,
	const char *function,
	const std::string &msg ) noexcept
	:
	KeyException{line, file, function, msg}
{

}

template<typename T>
std::string Game<T>::GameException::getType() const noexcept
{
	return typeid( this ).name();
}

template<typename T>
const char* Game<T>::GameException::what() const noexcept
{
	return KeyException::what();
}


Sandbox3d::Sandbox3d( const int width,
	const int height,
	const int x,
	const int y,
	const int nWindows )
	:
	Game(width, height, "KeyEngine 3d Sandbox", x, y, nWindows)
{
	auto &gph = m_mainWindow.getGraphics();

	s_cameraMan.setWidth( width );
	s_cameraMan.setHeight( height );
	s_cameraMan.add( std::make_unique<Camera>( gph, "A", width, height, 60.0f, dx::XMFLOAT3{0.0f, 0.0f, 0.0f}, 0.0f, util::PI / 2.0f, false, 0.5f, 1000.0f ) );
	s_cameraMan.add( std::make_unique<Camera>( gph, "B", width, height, 45.0f, dx::XMFLOAT3{-13.5f, 28.8f, -6.4f}, util::PI / 180.0f * 13.0f, util::PI / 180.0f * 61.0f, false, 0.5f, 1000.0f ) );
	m_pPointLight1 = std::make_unique<PointLight>( gph, dx::XMFLOAT3{10.0f, 5.0f, -1.4f} );
	//m_pPointLight2 = std::make_unique<PointLight>( m_mainWindow.getGraphics(), dx::XMFLOAT3{5.0f, 15.0f, 10.0f}, dx::XMFLOAT3{0.0f, 1.0f, 0.f}, false );
	//s_cameraMan.add( m_pPointLight1->shareCamera() );

	m_cube1.setPosition( {10.0f, 5.0f, 6.0f} );

	auto &renderer = gph.getRenderer();

	s_cameraMan.connectEffectsToRenderer( renderer );

	m_pPointLight1->connectEffectsToRenderer( renderer );
	//m_pPointLight2->connectEffectsToRenderer( renderer );

	m_terrain.connectEffectsToRenderer( renderer );
	m_cube1.connectEffectsToRenderer( renderer );
	m_cube2.connectEffectsToRenderer( renderer );
	m_cube3.connectEffectsToRenderer( renderer );
	m_testSphere.connectEffectsToRenderer( renderer );
	//m_sponzaScene.connectEffectsToRenderer( renderer );
	//m_nanoSuit.connectEffectsToRenderer( renderer );
	m_carabiner.connectEffectsToRenderer( renderer );

	m_cube2.setEffectEnabled( rch::blurOutline, false );

	if ( m_pPointLight1->isCastingShadows() )
	{
		gph.getRenderer3d().setShadowCamera( *m_pPointLight1->shareCamera(), true );
	}
	//if ( m_pPointLight2->isCastingShadows() )
	//{
	//	renderer.setShadowCamera( *m_pPointLight2->shareCamera(), true );
	//}

	/*
	ThreadPoolJ &threadPool = ThreadPoolJ::instance( 4u, true );
	threadPool.enqueue( &func_async::doPeriodically, &BindableMap::garbageCollect, 5000u, false );

	threadPool.enqueue( &func_async::doLater,
		[this]() -> void
		{
			this->m_testSphere.setRadius( 4.0f );
		},
		4000u );

	threadPool.enqueue( &func_async::doLater,
		[this]() -> void
		{
			this->m_testSphere.setRadius( 0.25f );
		},
		8000u );
	*/

	auto menuState = std::make_unique<MenuState>();
	setState( std::move( menuState ), m_mainWindow.getMouse() );
}

Sandbox3d::~Sandbox3d() noexcept
{

}

int Sandbox3d::loop()
{
	m_gameTimer.start();
	int returnC0de = -1;
	while ( true )
	{
		if ( const auto exitCode = m_mainWindow.messageLoop() )
		{
			return *exitCode;
		}

		const float dt = calcDt();
		returnC0de = checkInput( dt );
		if ( returnC0de == 0 )
		{
			break;
		}
		update( dt );
		render( dt );
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		present();
	}
	return returnC0de;
}

int Sandbox3d::checkInput( const float dt )
{
	auto &keyboard = m_mainWindow.getKeyboard();
	auto &mouse = m_mainWindow.getMouse();

	// process any keyboard events
	while ( const auto ev = keyboard.readEventQueue() )
	{
		if ( !ev->isPressEvent() )
		{
			continue;
		}

		switch( ev->getKeycode() )
		{
		case VK_ESCAPE:
		{
			if ( m_mainWindow.isCursorEnabled() )	// TODO: check the game state on how to respond not whether the cursor is enabled
			{
				setState( std::make_unique<GameState>(), mouse );
			}
			else
			{
				setState( std::make_unique<MenuState>(), mouse );
			}
			break;
		}
#ifndef FINAL_RELEASE
		case VK_F1:
		{
			m_bShowDemoWindow = true;
			break;
		}
#endif
		case VK_RETURN:
		{
			auto &gph = m_mainWindow.getGraphics();
			gph.getRenderer3d().dumpShadowMap( gph, "dumps/shadow_" );
			break;
		}
		}//switch
	}

	auto &activeCamera = s_cameraMan.activeCamera();
	if ( !m_mainWindow.isCursorEnabled() )
	{
		const float camSpeed = keyboard.isKeyPressed( VK_SHIFT ) ? 10.0f : 1.0f;

		if ( keyboard.isKeyPressed( 'W' ) )
		{
			activeCamera.translateRel( DirectX::XMFLOAT3{0.0f, 0.0f, dt * camSpeed} );
		}
		if ( keyboard.isKeyPressed( 'A' ) )
		{
			activeCamera.translateRel( {-dt * camSpeed, 0.0f, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'S' ) )
		{
			activeCamera.translateRel( {0.0f, 0.0f, -dt * camSpeed} );
		}
		if ( keyboard.isKeyPressed( 'D' ) )
		{
			activeCamera.translateRel( {dt * camSpeed, 0.0f, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'E' ) )
		{
			activeCamera.translateRel( {0.0f, dt * camSpeed, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'Q' ) )
		{
			activeCamera.translateRel( {0.0f, -dt * camSpeed, 0.0f} );
		}
		if ( keyboard.isKeyPressed( VK_BACK ) )
		{
			return 0;
		}
	}

	// Rotate Camera if in game mode
	while ( const auto &delta = mouse.readRawDeltaBuffer() )
	{
		if ( !m_mainWindow.isCursorEnabled() )
		{
			activeCamera.rotateRel( (float)delta->m_dx, (float)delta->m_dy );
		}
	}

	return 1;
}

void Sandbox3d::update( const float dt )
{
	const auto &activeCamera = s_cameraMan.getActiveCamera();
	// binds camera to all Passes that need it
	auto &gph = m_mainWindow.getGraphics();
	gph.getRenderer3d().setActiveCamera( activeCamera );

	m_pPointLight1->update( gph, dt, activeCamera.getViewMatrix() );
	//m_pPointLight2->update( gph, dt, activeCamera.getViewMatrix() );

	m_terrain.update( dt );
	m_cube1.update( dt );
	m_cube2.update( dt );
	m_cube3.update( dt );
	//m_nanoSuit.update( dt );
	m_carabiner.update( dt );
	//m_sponzaScene.update( dt );
}

void Sandbox3d::render( const float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginFrame();

	m_pPointLight1->render( rch::lambert );
	//m_pPointLight2->render( rch::lambert );

	m_terrain.render();
	m_cube1.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_cube2.render();
	m_cube3.render();
	m_testSphere.render();
	//m_nanoSuit.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_carabiner.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	//m_sponzaScene.render( rch::lambert | rch::shadow );

	s_cameraMan.render( rch::lambert | rch::wireframe );

	gph.runRenderer();
}

void Sandbox3d::test()
{
#ifndef FINAL_RELEASE
	using namespace std::string_literals;
	KeyConsole &console = KeyConsole::getInstance();

	//const BindableMap &instanceToBeInspected = BindableMap::getInstance();

	//console.print( "BindableMap instance count: "s + std::to_string( BindableMap::getInstanceCount() ) + "\n"s );
	//console.print( "BindableMap garbage count: "s + std::to_string( BindableMap::getGarbageCount() ) + "\n"s );

	console.print( "Current distance from carabiner: "s + std::to_string( m_carabiner.getDistanceFromActiveCamera() ) + "\n"s );

	/// Render Imgui stuff
	auto &gph = m_mainWindow.getGraphics();

	// Showcase Effect controls by passing visitors to the object hierarchies
	s_cameraMan.displayImguiWidgets( gph );

	m_pPointLight1->displayImguiWidgets();
	//m_pPointLight2->displayImguiWidgets();

	m_terrain.displayImguiWidgets( gph, "Terrain Base"s );

	m_cube1.displayImguiWidgets( gph, "Cube 1"s );
	m_cube2.displayImguiWidgets( gph, "Cube 2"s );
	m_cube3.displayImguiWidgets( gph, "Cube 3"s );

	m_carabiner.displayImguiWidgets( gph );

	gph.getRenderer3d().displayImguiWidgets( gph );

	if ( m_bShowDemoWindow )
	{
		ImGui::ShowDemoWindow( &m_bShowDemoWindow );
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Arkanoid::Arkanoid( const int width,
	const int height,
	const int x,
	const int y )
	:
	Game(width, height, "Arkanoid", x, y),
	m_ball(dx::XMFLOAT2{450.0f, 450.0f}, dx::XMFLOAT2{-300.0f, -300.0f}),
	m_walls(Rect(0.0f, (float)width, 0.0f, (float)height)),
	m_paddle(dx::XMFLOAT2(400.0f, 550.0f), 40.0f, 8.0f, col::Cyan, col::Orange),
	m_brickSound("assets/sfx/arkanoid_brick.wav", "Arkanoid Brick"),
	m_padSound("assets/sfx/arkanoid_pad.wav", "Arkanoid Pad")
{
	// set bricks starting positions and assign different color for each brick row
	const ColorBGRA colors[4] = {col::Red, col::Green, col::Blue, col::Gold};
	const dx::XMFLOAT2 topLeft{40.0f, 40.0f};

	for ( int i = 0, yi = 0; yi < s_nBricksVertically; ++yi )
	{
		ColorBGRA rowCol = colors[yi];
		for ( int xi = 0; xi < s_nBricksHorizontally; ++xi )
		{
			auto curBrickTopLeftOffset = dx::XMFLOAT2{xi * s_brickWidth, yi * s_brickHeight};
			m_bricks[i] = Brick{Rect{dx::XMFLOAT2{topLeft.x + curBrickTopLeftOffset.x, topLeft.y + curBrickTopLeftOffset.y}, s_brickWidth, s_brickHeight}, rowCol};
			++i;
		}
	}
}

int Arkanoid::loop()
{
	int returnC0de = -1;
	m_gameTimer.start();
	while ( true )
	{
		if ( const auto exitCode = m_mainWindow.messageLoop() )
		{
			return *exitCode;
		}

		const float dt = calcDt();
		returnC0de = checkInput( dt );
		if ( returnC0de == 0 )
		{
			break;
		}
		update( dt );
		render( dt );
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		present();
	}
	return returnC0de;
}

int Arkanoid::checkInput( const float dt )
{
	auto &keyboard = m_mainWindow.getKeyboard();
	auto &mouse = m_mainWindow.getMouse();

	// process any keyboard events
	while ( const auto ev = keyboard.readEventQueue() )
	{
		if ( !ev->isPressEvent() )
		{
			continue;
		}

		switch( ev->getKeycode() )
		{
		case VK_ESCAPE:
		{
			if ( m_mainWindow.isCursorEnabled() )
			{
				setState( std::make_unique<GameState>(), mouse );
			}
			else
			{
				setState( std::make_unique<MenuState>(), mouse );
			}
			break;
		}
		}
	}

	if ( keyboard.isKeyPressed( VK_LEFT ) )
	{
		m_paddle.setPositionRel( -s_speed * dt );
	}
	if ( keyboard.isKeyPressed( VK_RIGHT ) )
	{
		m_paddle.setPositionRel( s_speed * dt );
	}
	if ( keyboard.isKeyPressed( VK_BACK ) )
	{
		return 0;
	}
	return 1;
}

void Arkanoid::update( const float dt )
{
	m_ball.update( dt );
	m_paddle.doWallCollision( m_walls );

	bool bCollided = false;
	float currentColDist = 999999.999f;
	int curColBrickIndex;
	for ( int i = 0; i < s_nBricks; ++i )
	{
		if ( m_bricks[i].checkForBallCollision( m_ball ) )
		{
			float newCollisionDistance;
			const auto brickCenter = m_bricks[i].calcCenter();
			dx::XMStoreFloat( &newCollisionDistance, dx::XMVector2LengthSq( dx::XMVectorSubtract( dx::XMLoadFloat2( &m_ball.getPosition() ), dx::XMLoadFloat2( &brickCenter ) ) ) );
			if ( bCollided )
			{// limit to one collision per frame
				if ( newCollisionDistance < currentColDist )
				{
					currentColDist = newCollisionDistance;
					curColBrickIndex = i;
				}
			}
			else
			{
				currentColDist = newCollisionDistance;
				curColBrickIndex = i;
				bCollided = true;
			}
		}
	}
	if ( bCollided )
	{
		m_paddle.resetCollisionCooldown();
		m_bricks[curColBrickIndex].doBallCollision( m_ball );
		m_brickSound.play();
	}
	// end ball collision with bricks

	if ( m_paddle.doBallCollision( m_ball ) )
	{
		m_padSound.play();
	}
	if ( m_ball.doWallCollision( m_walls ) )
	{
		m_paddle.resetCollisionCooldown();
		m_padSound.play();
	}
}

void Arkanoid::render( const float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginFrame();

	m_ball.render( gph );
	for ( const Brick &b : m_bricks )
	{
		b.render( gph );
	}
	m_paddle.render( gph );

	gph.runRenderer();
}

void Arkanoid::test()
{
#if defined _DEBUG && !defined NDEBUG
	pass_;
#endif
}