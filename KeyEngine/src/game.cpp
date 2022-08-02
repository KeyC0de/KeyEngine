#include <algorithm>
#include "game.h"
#include "imgui_manager.h"
#include "math_utils.h"
#include "imgui.h"
#include "utils.h"
#include "effect_visitor.h"
#include "model_visitor.h"
#include "camera.h"
#include "rendering_channel.h"
#include "console.h"
#include "assertions_console.h"
#include "drawable.h"
#include "graphics_mode.h"
//#include "../../KeyEngine_tests/testing.h"


namespace dx = DirectX;

template <typename T>
Game<T>::Game( int width,
	int height,
	const std::string &title,
	unsigned nWindows )
	:
	m_pImguiMan{createImgui()},
	m_mainWindow{width, height, title.c_str()},
	m_pCurrentState{std::make_unique<GameState>()}
{
	m_nWindows = nWindows;
}

template <typename T>
ImguiManager* Game<T>::createImgui() noexcept
{
	if constexpr ( GraphicsMode::get() == GraphicsMode::_3D )
	{
		return ImguiManager::getInstance();
	}
	else
	{
		return nullptr;
	}
}

template <typename T>
std::optional<Window*> Game<T>::getForegroundWindow() noexcept
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
		console.print( "Game state is on!" );
#endif
	}
	else if ( dynamic_cast<MenuState*>( pNewState.get() ) != nullptr )
	{
		m_mainWindow.enableCursor();
		mouse.disableRawInput();
#if defined _DEBUG && !defined NDEBUG
		KeyConsole &console = KeyConsole::getInstance();
		console.print( "Menu state is on!" );
#endif
	}
	else
	{
		throwGameException( "Invalid state!" );
	}
	m_pCurrentState.swap( pNewState );
}

template <typename T>
State* Game<T>::getState() noexcept
{
	return m_pCurrentState.get();
}

template <typename T>
float Game<T>::calculateDt()
{
	auto &settings = m_settingsMan.accessSettings();
	static float minFrameTime = 1.0f / settings.iMaxFps;
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
Game<T>::GameException::GameException( int line,
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


Sandbox3d::Sandbox3d( int width,
	int height,
	int nWindows )
	:
	Game(width, height, "KeyEngine 3d Sandbox", nWindows),
	m_renderer{m_mainWindow.getGraphics(), 4, 3.0f}
{
	m_cameraMan.setWidthHeight( width,
		height );
	m_cameraMan.add( std::make_unique<Camera>( m_mainWindow.getGraphics(),
		"A",
		width,
		height,
		60.0f,
		dx::XMFLOAT3{-13.5f, 6.0f, 3.5f},
		0.0f,
		util::PI / 2.0f) );
	m_cameraMan.add( std::make_unique<Camera>( m_mainWindow.getGraphics(),
		"B",
		width,
		height,
		45.0f,
		dx::XMFLOAT3{-13.5f, 28.8f, -6.4f},
		util::PI / 180.0f * 13.0f,
		util::PI / 180.0f * 61.0f ) );
	m_pPointLight1 = std::make_unique<PointLight>( m_mainWindow.getGraphics(),
		dx::XMFLOAT3{10.0f, 5.0f, 0.0f} );
	//m_pPointLight2 = std::make_unique<PointLight>( m_mainWindow.getGraphics(),
	//	dx::XMFLOAT3{5.0f, 15.0f, 10.0f}, dx::XMFLOAT3{0.0f, 1.0f, 0.f}, false );
	//m_cameraMan.add( m_pPointLight1->shareCamera() );

	m_cube1.setWorldPosition( {10.0f, 5.0f, 6.0f} );

	m_nanoSuit.setTransform( dx::XMMatrixRotationY( util::PI / 2.f ) *
		dx::XMMatrixTranslation( 27.f, -0.56f, 1.7f ) );
	m_carabiner.setTransform( dx::XMMatrixTranslation( -10.0f, 6.0f, 0.0f ) );

	m_pPointLight1->connectEffectsToRenderer( m_renderer );
	//m_pPointLight2->connectEffectsToRenderer( m_renderer );

	m_cube1.connectEffectsToRenderer( m_renderer );
	m_cube2.connectEffectsToRenderer( m_renderer );
	m_sponzaScene.connectEffectsToRenderer( m_renderer );
	m_nanoSuit.connectEffectsToRenderer( m_renderer );
	m_carabiner.connectEffectsToRenderer( m_renderer );
	m_cameraMan.connectEffectsToRenderer( m_renderer );

	if ( m_pPointLight1->isCastingShadows() )
	{
		m_renderer.setShadowCamera( *m_pPointLight1->shareCamera() );
	}
	//if ( m_pPointLight2->isCastingShadows() )
	//{
	//	m_renderer.setShadowCamera( *m_pPointLight2->shareCamera() );
	//}

	auto menuState = std::make_unique<MenuState>();
	setState( std::move( menuState ),
		m_mainWindow.getMouse() );
}


int Sandbox3d::loop()
{
	m_gameTimer.start();
	while ( true )
	{
		if ( const auto exitCode = m_mainWindow.messageLoop() )
		{
			return *exitCode;
		}

		const float dt = calculateDt();
		checkInput( dt );
		// checkScripts()
		update( dt );
		render( dt );
		present();
	}
	return -1;
}

void Sandbox3d::checkInput( float dt )
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
			m_renderer.dumpShadowMap( m_mainWindow.getGraphics(),
				"dumps/shadow_" );
			break;
		}
		}//switch
	}

	auto &activeCamera = m_cameraMan.getActiveCamera();
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
}

void Sandbox3d::update( float dt )
{
	auto &activeCamera = m_cameraMan.getActiveCamera();
	// binds camera to all Passes that need it
	m_renderer.setMainCamera( activeCamera );
	auto &gph = m_mainWindow.getGraphics();
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

void Sandbox3d::render( float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginRendering();

	m_pPointLight1->render( rch::lambert );
	//m_pPointLight2->render( rch::lambert );

	m_cube1.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	m_cube2.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	m_nanoSuit.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_carabiner.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	m_sponzaScene.render( rch::lambert | rch::shadow );

	m_cameraMan.render( rch::lambert );

	m_renderer.run( gph );

	renderImgui();
	gph.updateAndRenderFpsTimer();
}

void Sandbox3d::renderImgui()
{
	auto &gph = m_mainWindow.getGraphics();

	// Showcase Effect controls by passing visitors to the object hierarchies
	static MV sponzaVisitor{"Sponza"};
	static MV nanoSuitVisitor{"Nano"};
	static MV carabinerVisitor{"Carabiner"};
	sponzaVisitor.spawnModelImgui( m_sponzaScene );
	nanoSuitVisitor.spawnModelImgui( m_nanoSuit );
	carabinerVisitor.spawnModelImgui( m_carabiner );
	m_cameraMan.spawnImguiWindow( gph );
	m_pPointLight1->displayImguiWidgets();
	//m_pPointLight2->displayImguiWidgets();
	m_cube1.displayImguiWidgets( gph, "Cube 1" );
	m_cube2.displayImguiWidgets( gph, "Cube 2" );
	m_renderer.showImGuiWindows( gph );

	if ( b_bShowDemoWindow )
	{
		ImGui::ShowDemoWindow( &b_bShowDemoWindow );
	}
}

void Sandbox3d::present()
{
	auto &gph = m_mainWindow.getGraphics();
	gph.endRendering();
	m_renderer.reset();
}


Arkanoid::Arkanoid( int width,
	int height )
	:
	Game(width, height, "Arkanoid"),
	m_renderer{m_mainWindow.getGraphics()},
	m_ball(dx::XMFLOAT2{450.0f, 450.0f}, dx::XMFLOAT2{-300.0f, -300.0f}),
	m_walls(Rect(0.0f, (float)width, 0.0f, (float)height)),
	m_paddle(dx::XMFLOAT2(400.0f, 550.0f), 40.0f, 8.0f, col::Cyan, col::Orange),
	m_brickSound("assets/sfx/arkanoid_brick.wav", "Arkanoid Brick"),
	m_padSound("assets/sfx/arkanoid_pad.wav", "Arkanoid Pad")
{
	// set bricks starting positions and assign different color for each brick row
	const ColorBGRA colors[4] = {col::Red, col::Green, col::Blue, col::Gold};
	const dx::XMFLOAT2 topLeft{40.0f, 40.0f};

	for ( int i = 0, y = 0; y < m_nBricksVertically; ++y )
	{
		ColorBGRA rowCol = colors[y];
		for ( int x = 0; x < m_nBricksHorizontally; ++x )
		{
			auto curBrickTopLeftOffset = dx::XMFLOAT2{x * m_brickWidth, y * m_brickHeight};
			m_bricks[i] = Brick{Rect{dx::XMFLOAT2{topLeft.x + curBrickTopLeftOffset.x,
									topLeft.y + curBrickTopLeftOffset.y},
					m_brickWidth, m_brickHeight},
				rowCol};
			++i;
		}
	}
}

int Arkanoid::loop()
{
	m_gameTimer.start();
	while ( true )
	{
		if ( const auto exitCode = m_mainWindow.messageLoop() )
		{
			return *exitCode;
		}

		const float dt = calculateDt();
		checkInput( dt );
		update( dt );
		render( dt );
		present();
	}
	return -1;
}

void Arkanoid::checkInput( float dt )
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
		m_paddle.setPositionRel( -m_speed * dt );
	}
	if ( keyboard.isKeyPressed( VK_RIGHT ) )
	{
		m_paddle.setPositionRel( m_speed * dt );
	}
}

void Arkanoid::update( float dt )
{
	m_ball.update( dt );
	m_paddle.doWallCollision( m_walls );

	bool bCollided = false;
	float currentColDist;
	int curColBrickIndex;
	for ( int i = 0; i < m_nBricks; ++i )
	{
		if ( m_bricks[i].checkForBallCollision( m_ball ) )
		{
			float newCollisionDistance;
			dx::XMStoreFloat( &newCollisionDistance,
				dx::XMVector2LengthSq( dx::XMVectorSubtract( dx::XMLoadFloat2( &m_ball.getPosition() ),
						dx::XMLoadFloat2( &m_bricks[i].getCenter() ) ) ) );
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
		m_paddle.resetCooldown();
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
		m_paddle.resetCooldown();
		m_padSound.play();
	}
}

void Arkanoid::render( float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginRendering();

	m_ball.render( gph );
	for ( const Brick &b : m_bricks )
	{
		b.render( gph );
	}
	m_paddle.render( gph );

	m_renderer.run( gph );
}

void Arkanoid::present()
{
	auto &gph = m_mainWindow.getGraphics();
	gph.endRendering();
	m_renderer.reset();
}


/*Snake::Snake( int width,
	int height )
	:
	Game(width, height, "SnakeRepr"),
	m_renderer{m_mainWindow.getGraphics()},
	rng(std::random_device()()),	// initialize random uniformly-
	board(graphics, 100, 100, window.windowClientHeight - 200, window.windowClientHeight - 200),
	snek({ 220 / board.dimension, 220 / board.dimension }),
	fruit(rng, board, snek)
{

}

int Snake::loop()
{
	m_gameTimer.start();
	while ( true )
	{
		if ( const auto exitCode = m_mainWindow.messageLoop() )
		{
			return *exitCode;
		}

		const float dt = calculateDt();
		checkInput( dt );
		update( dt );
		render( dt );
		present();
	}
	return -1;
}

void Snake::checkInput( float dt )
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

	if( window.keyboard.isKeyPressed( VK_UP ) )
	{
		const Location2d new_delta_loc = { 0,-1 };
		if( delta_loc != -new_delta_loc || snek.getLength() <= 2 )
		{
			delta_loc = new_delta_loc;
		}
	}
	else if( window.keyboard.isKeyPressed( VK_DOWN ) )
	{
		const Location2d new_delta_loc = { 0,1 };
		if( delta_loc != -new_delta_loc || snek.getLength() <= 2 )
		{
			delta_loc = new_delta_loc;
		}
	}
	else if( window.keyboard.isKeyPressed( VK_LEFT ) )
	{
		const Location2d new_delta_loc = { -1,0 };
		if( delta_loc != -new_delta_loc || snek.getLength() <= 2 )
		{
			delta_loc = new_delta_loc;
		}
	}
	else if( window.keyboard.isKeyPressed( VK_RIGHT ) )
	{
		const Location2d new_delta_loc = { 1,0 };
		if( delta_loc != -new_delta_loc || snek.getLength() <= 2 )
		{
			delta_loc = new_delta_loc;
		}
	}

	m_snekModifiedMovePeriod = snakeMovePeriod;
	if( window.keyboard.isKeyPressed( VK_CONTROL ) )
	{
		m_snekModifiedMovePeriod = std::min( snakeMovePeriod,howManyFruitsForSpeedUp );
	}

}

void Snake::update( float dt )
{
	if (!gameIsOver)
	{
		if (window.keyboard.isKeyPressed(VK_UP))
		{
			delta_loc = { 0, -1 };
		}
		if (window.keyboard.isKeyPressed(VK_DOWN))
		{
			delta_loc = { 0, 1 };
		}
		if (window.keyboard.isKeyPressed(VK_LEFT))
		{
			delta_loc = { -1, 0 };
		}
		if (window.keyboard.isKeyPressed(VK_RIGHT))
		{
			delta_loc = { 1, 0 };
		}

		++snakeMoveCounter;
		if (snakeMoveCounter >= snakeMovePeriod)
		{
			snakeMoveCounter = 0;
			Location2d nextLoc = snek.getNextHeadLocation(delta_loc);
			if (!board.isInsideBoard(nextLoc) || snek.hasCollided(nextLoc))
			{
				gameIsOver = true;
			}
			else
			{
				const bool eating = nextLoc == fruit.getLocation();
				if (eating)
				{
					snek.grow(delta_loc);
					fruitsEaten++;
					if (fruitsEaten % nHowManyFruits == 0)
					{
						snakeMovePeriod--;
						if (snakeMovePeriod < 6)
							snakeMovePeriod = 6;
					}
				}
				snek.moveBy(delta_loc);
				if (eating) // if eaten generate new location of fruit
				{
					fruit.respawn(rng, board, snek);
				}
			}
		}
	}
}

void Snake::render( float dt )
{
	auto &gph = m_mainWindow.getGraphics();
	gph.beginRendering();


	m_renderer.run( gph );
}

void Snake::present()
{
	auto &gph = m_mainWindow.getGraphics();
	gph.endRendering();
	m_renderer.reset();
}*/