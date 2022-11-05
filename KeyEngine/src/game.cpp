#include "game.h"
#include <algorithm>
#include "math_utils.h"
#include "imgui.h"
#include "effect_visitor.h"
#include "model_visitor.h"
#include "camera.h"
#include "rendering_channel.h"
#include "console.h"
#include "assertions_console.h"
#include "mesh.h"
#include "graphics_mode.h"
#include "thread_pool_j.h"
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
	const unsigned nWindows )
	:
	m_pImguiMan{createImgui()},
	m_mainWindow{width, height, title.c_str(), MAIN_WINDOW_CLASS_NAME, Window::windowProc, 200, 200, ColorBGRA{255, 255, 255}, LoadMenuW( THIS_INSTANCE, MAKEINTRESOURCEW( IDR_MENU_APP ) )},
	m_pCurrentState{std::make_unique<GameState>()}
{
	s_nWindows = nWindows;
}

template <typename T>
ImguiManager* Game<T>::createImgui() noexcept
{
	if constexpr ( gph_mode::get() == gph_mode::_3D )
	{
		return new ImguiManager{};
	}
	else
	{
		return nullptr;
	}
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
		KeyConsole &console = KeyConsole::instance();
		console.print( "\nGame state is on!\n" );
#endif
	}
	else if ( dynamic_cast<MenuState*>( pNewState.get() ) != nullptr )
	{
		m_mainWindow.enableCursor();
		mouse.disableRawInput();
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::instance();
		console.print( "\nMenu state is on!\n" );
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
State& Game<T>::state() noexcept
{
	return m_pCurrentState.get();
}

template <typename T>
const float Game<T>::calcDt()
{
	auto &settings = s_settingsMan.settings();
	static float minFrameTime = 1.0f / settings.iMaxFps;
	float dt = m_gameTimer.lap() * settings.fGameSpeed;

#if defined _DEBUG && !defined NDEBUG
	// print frame time
	KeyConsole &console = KeyConsole::instance();
	using namespace std::string_literals;
	++settings.frameCount;
	std::string frameStats = "Frame time : "s
		+ std::to_string( dt )
		+ "ms. Frame "s
		+ std::to_string( settings.frameCount )
		+ "\n"s;
	console.print( std::move( frameStats ) );
#endif // _DEBUG
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
const std::string Game<T>::GameException::getType() const noexcept
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
	const int nWindows )
	:
	Game(width, height, "KeyEngine 3d Sandbox", nWindows)
{
	auto &gph = m_mainWindow.getGraphics();

	s_cameraMan.setWidth( width );
	s_cameraMan.setHeight( height );
	s_cameraMan.add( std::make_unique<Camera>( gph,
		"A",
		width,
		height,
		60.0f,
		dx::XMFLOAT3{-13.5f, 6.0f, 3.5f},
		0.0f,
		util::PI / 2.0f) );
	s_cameraMan.add( std::make_unique<Camera>( gph,
		"B",
		width,
		height,
		45.0f,
		dx::XMFLOAT3{-13.5f, 28.8f, -6.4f},
		util::PI / 180.0f * 13.0f,
		util::PI / 180.0f * 61.0f ) );
	m_pPointLight1 = std::make_unique<PointLight>( gph,
		dx::XMFLOAT3{10.0f, 5.0f, 0.0f} );
	//m_pPointLight2 = std::make_unique<PointLight>( m_mainWindow.getGraphics(),
	//	dx::XMFLOAT3{5.0f, 15.0f, 10.0f}, dx::XMFLOAT3{0.0f, 1.0f, 0.f}, false );
	//s_cameraMan.add( m_pPointLight1->shareCamera() );

	m_cube1.setPosition( {10.0f, 5.0f, 6.0f} );

	m_nanoSuit.setRootTransform( dx::XMMatrixRotationY( util::PI / 2.f ) *
		dx::XMMatrixTranslation( 27.f, -0.56f, 1.7f ) );
	m_carabiner.setRootTransform( dx::XMMatrixTranslation( -10.0f, 6.0f, 0.0f ) );

	auto &renderer = gph.renderer();
	m_pPointLight1->connectEffectsToRenderer( renderer );
	//m_pPointLight2->connectEffectsToRenderer( renderer );

	m_cube1.connectEffectsToRenderer( renderer );
	m_cube2.connectEffectsToRenderer( renderer );
	m_testSphere.connectEffectsToRenderer( renderer );
	m_sponzaScene.connectEffectsToRenderer( renderer );
	m_nanoSuit.connectEffectsToRenderer( renderer );
	m_carabiner.connectEffectsToRenderer( renderer );
	//s_cameraMan.connectEffectsToRenderer( renderer );

	m_cube2.setEffectEnabled( rch::blurOutline,
		false );

	if ( m_pPointLight1->isCastingShadows() )
	{
		gph.renderer3d().setShadowCamera( *m_pPointLight1->shareCamera() );
	}
	//if ( m_pPointLight2->isCastingShadows() )
	//{
	//	renderer.setShadowCamera( *m_pPointLight2->shareCamera() );
	//}

	ThreadPoolJ &threadPool = ThreadPoolJ::instance( 4u,
		true );
	threadPool.enqueue( &func_async::doPeriodically,
		&BindableMap::garbageCollect,
		5000u,
		false );

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

	auto menuState = std::make_unique<MenuState>();
	setState( std::move( menuState ),
		m_mainWindow.mouse() );
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
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		render( dt );
		present();
	}
	return returnC0de;
}

int Sandbox3d::checkInput( const float dt )
{
	auto &keyboard = m_mainWindow.keyboard();
	auto &mouse = m_mainWindow.mouse();

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
				setState( std::make_unique<GameState>(),
					mouse );
			}
			else
			{
				setState( std::make_unique<MenuState>(),
					mouse );
			}
			break;
		}
		case VK_F1:
		{
			b_bShowDemoWindow = true;
			break;
		}
		case VK_RETURN:
		{
			auto &gph = m_mainWindow.getGraphics();
			gph.renderer3d().dumpShadowMap( gph,
				"dumps/shadow_" );
			break;
		}
		}//switch
	}

	auto &activeCamera = s_cameraMan.activeCamera();
	if ( !m_mainWindow.isCursorEnabled() )
	{
		if ( keyboard.isKeyPressed( 'W' ) )
		{
			activeCamera.translateRel( {0.0f, 0.0f, dt} );
		}
		if ( keyboard.isKeyPressed( 'A' ) )
		{
			activeCamera.translateRel( {-dt, 0.0f, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'S' ) )
		{
			activeCamera.translateRel( {0.0f, 0.0f, -dt} );
		}
		if ( keyboard.isKeyPressed( 'D' ) )
		{
			activeCamera.translateRel( {dt, 0.0f, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'E' ) )
		{
			activeCamera.translateRel( {0.0f, dt, 0.0f} );
		}
		if ( keyboard.isKeyPressed( 'Q' ) )
		{
			activeCamera.translateRel( {0.0f, -dt, 0.0f} );
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
			activeCamera.rotateRel( (float)delta->m_dx,
				(float)delta->m_dy );
		}
	}

	return 1;
}

void Sandbox3d::update( const float dt )
{
	auto &activeCamera = s_cameraMan.activeCamera();
	// binds camera to all Passes that need it
	auto &gph = m_mainWindow.getGraphics();
	gph.renderer3d().setActiveCamera( activeCamera );
	/*
	m_world.update( dt );		// updates current level-map stuff, terrain, weather, world globals etc.
	// actor manager updates actors which also updates AI
	m_actorManager.update( dt );	// updates actors - should be much more efficient after spatial partitioning and frustum culling - for rendering too
	spatialPartitionGameObjects();		// actors + statics
	frustumCullGameObjects();			// actors + statics - 1 frustum for each renderable viewpoint (typically just the player's camera)

	m_physics.update( CONST_DT );
	m_playerManager.update( dt );
	m_skybox.update( dt );
	m_lightManager.update( dt );	// update scene lights
	m_cameraManager.update( dt );	// update active cameras
	m_particles.update( dt );
	m_soundManager.update( dt );
	*/
	m_pPointLight1->update( gph,
		dt,
		activeCamera.getViewMatrix() );
	//m_pPointLight2->update( gph,
	//	dt,
	//	activeCamera.getViewMatrix() );

	m_cube1.update( dt );
	m_cube2.update( dt );
	m_nanoSuit.update( dt );
	m_carabiner.update( dt );
	m_sponzaScene.update( dt );
}

void Sandbox3d::render( const float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginFrame();

	m_pPointLight1->render( rch::lambert );
	//m_pPointLight2->render( rch::lambert );

	m_cube1.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_cube2.render();
	m_testSphere.render();
	m_nanoSuit.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_carabiner.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	m_sponzaScene.render( rch::lambert | rch::shadow );

	s_cameraMan.render( rch::lambert | rch::wireframe );

	gph.runRenderer();

	gph.updateAndRenderFpsTimer();
}

#if defined _DEBUG && !defined NDEBUG
void Sandbox3d::test()
{
	using namespace std::string_literals;
	KeyConsole &console = KeyConsole::instance();

	const BindableMap &instanceToBeInspected = BindableMap::getInstance();

	console.print( "BindableMap instance count: "s + std::to_string( BindableMap::getInstanceCount() ) + "\n"s );
	console.print( "BindableMap garbage count: "s + std::to_string( BindableMap::getGarbageCount() ) + "\n"s );
	console.print( "Current distance from carabiner: "s + std::to_string( m_carabiner.getDistanceFromActiveCamera() ) + "\n"s );

	/// Render Imgui stuff
	auto &gph = m_mainWindow.getGraphics();

	// Showcase Effect controls by passing visitors to the object hierarchies
	static ImguiVisitor sponzaVisitor{"Sponza"};
	static ImguiVisitor nanoSuitVisitor{"Nano"};
	static ImguiVisitor carabinerVisitor{"Carabiner"};
	sponzaVisitor.spawnModelImgui( m_sponzaScene );
	nanoSuitVisitor.spawnModelImgui( m_nanoSuit );
	carabinerVisitor.spawnModelImgui( m_carabiner );
	s_cameraMan.spawnImguiWindow( gph );
	m_pPointLight1->displayImguiWidgets();
	//m_pPointLight2->displayImguiWidgets();
	m_cube1.displayImguiWidgets( gph, "Cube 1" );
	m_cube2.displayImguiWidgets( gph, "Cube 2" );
	gph.renderer3d().showImGuiWindows( gph );

	if ( b_bShowDemoWindow )
	{
		ImGui::ShowDemoWindow( &b_bShowDemoWindow );
	}
}
#endif


Arkanoid::Arkanoid( const int width,
	const int height )
	:
	Game(width, height, "Arkanoid"),
	m_ball(dx::XMFLOAT2{450.0f, 450.0f}, dx::XMFLOAT2{-300.0f, -300.0f}),
	m_walls(Rect(0.0f, (float)width, 0.0f, (float)height)),
	m_paddle(dx::XMFLOAT2(400.0f, 550.0f), 40.0f, 8.0f, col::Cyan, col::Orange),
	m_brickSound("assets/sfx/arkanoid_brick.wav", "Arkanoid Brick"),
	m_padSound("assets/sfx/arkanoid_pad.wav", "Arkanoid Pad")
{
	// set bricks starting positions and assign different color for each brick row
	const ColorBGRA colors[4] = {col::Red, col::Green, col::Blue, col::Gold};
	const dx::XMFLOAT2 topLeft{40.0f, 40.0f};

	for ( int i = 0, y = 0; y < s_nBricksVertically; ++y )
	{
		ColorBGRA rowCol = colors[y];
		for ( int x = 0; x < s_nBricksHorizontally; ++x )
		{
			auto curBrickTopLeftOffset = dx::XMFLOAT2{x * s_brickWidth, y * s_brickHeight};
			m_bricks[i] = Brick{Rect{dx::XMFLOAT2{topLeft.x + curBrickTopLeftOffset.x,
									topLeft.y + curBrickTopLeftOffset.y},
					s_brickWidth, s_brickHeight},
				rowCol};
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
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		render( dt );
		present();
	}
	return returnC0de;
}

int Arkanoid::checkInput( const float dt )
{
	auto &keyboard = m_mainWindow.keyboard();
	auto &mouse = m_mainWindow.mouse();

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
				setState( std::make_unique<GameState>(),
					mouse );
			}
			else
			{
				setState( std::make_unique<MenuState>(),
					mouse );
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
			dx::XMStoreFloat( &newCollisionDistance,
				dx::XMVector2LengthSq( dx::XMVectorSubtract( dx::XMLoadFloat2( &m_ball.getPosition() ),
					dx::XMLoadFloat2( &brickCenter ) ) ) );
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

#if defined _DEBUG && !defined NDEBUG
void Arkanoid::test()
{

}
#endif