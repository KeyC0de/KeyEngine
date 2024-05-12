#include "game.h"
#include "reporter_access.h"
#include <algorithm>
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
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
	//IListener<SwapChainResized>(),
#ifndef FINAL_RELEASE
	m_pImguiMan{createImgui()},
#endif
	m_mainWindow{width, height, title.c_str(), MAIN_WINDOW_CLASS_NAME, Window::windowProc, x, y, ColorBGRA{255, 255, 255}, LoadMenuW( THIS_INSTANCE, MAKEINTRESOURCEW( IDR_MENU_APP ) )},
	m_pCurrentState{std::make_unique<GameState>()}
{
	s_nWindows = nWindows;
	m_gameTimer.start();

	//static_cast<const IReporter<SwapChainResized>&>( *this ).addListener( this );
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

template<typename T>
void Game<T>::present()
{
	auto &gfx = m_mainWindow.getGraphics();
	gfx.endFrame();
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
	Game(width, height, "KeyEngine 3d Sandbox", x, y, nWindows),
	IListener<SwapChainResized>()
{
	auto &gfx = m_mainWindow.getGraphics();

	s_cameraMan.add( std::make_unique<Camera>( gfx, "A", width, height, 60.0f, dx::XMFLOAT3{0.0f, 0.0f, 0.0f}, 0.0f, util::PI / 2.0f, false, 0.5f, 1000.0f ) );
	s_cameraMan.add( std::make_unique<Camera>( gfx, "B", width, height, 45.0f, dx::XMFLOAT3{-13.5f, 28.8f, -6.4f}, util::PI / 180.0f * 13.0f, util::PI / 180.0f * 61.0f, false, 0.5f, 1000.0f ) );
	m_pPointLight1 = std::make_unique<PointLight>( gfx, dx::XMFLOAT3{10.0f, 5.0f, -1.4f} );
	//m_pPointLight2 = std::make_unique<PointLight>( m_mainWindow.getGraphics(), dx::XMFLOAT3{5.0f, 15.0f, 10.0f}, dx::XMFLOAT3{0.0f, 1.0f, 0.f}, false );

	m_cube1.setPosition( {10.0f, 5.0f, 6.0f} );

	auto &renderer = gfx.getRenderer3d();

	connectToRenderer( renderer );

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

	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<const IReporter<SwapChainResized>&>( reportingNexus ).addListener( this );
}

Sandbox3d::~Sandbox3d() noexcept
{

}

void Sandbox3d::notify( const SwapChainResized &event )
{
	(void)event;

	connectToRenderer( m_mainWindow.getGraphics().getRenderer3d() );
}

int Sandbox3d::loop()
{
	auto &settings = s_settingsMan.getSettings();

	int returnC0de = -1;

	bool bActive = true;
	float dt;

	auto runFixedLoop = [this] ( const float dt ) -> float
		{
			static constexpr int fixedUpdatesPerSecond = 60;
			static constexpr float dtFixed = 1.0f / fixedUpdatesPerSecond;
			static constexpr int maxFramesSkip = 10;	// if fps drops below fixedUpdatesPerSecond / maxFramesSkip the actual game will slow down

			float accumulator = 0.0f;
			accumulator += dt;
			accumulator = std::min( accumulator, 2.0f );	// prevent huge delta times

			float loops = 0;
			while ( accumulator >= dtFixed && loops < maxFramesSkip )
			{
				updateFixed( dtFixed );
				accumulator -= dtFixed;
				++loops;
			}

			return accumulator / dtFixed;
		};

	while ( true )
	{
		auto msgCode = m_mainWindow.messageLoop();
		if ( msgCode )
		{
			if ( *msgCode == 0 )
			{
				return 0;
			}
		}

		if ( bActive )
		{
			dt = m_gameTimer.lap() * settings.fGameSpeed;
			returnC0de = checkInput( dt );
			if ( returnC0de == 0 )
			{
				break;
			}
			float lerpBetweenFrames = runFixedLoop( dt );
			update( dt, lerpBetweenFrames );
			render();

#ifndef FINAL_RELEASE
			using namespace std::string_literals;
			KeyConsole &console = KeyConsole::getInstance();
			++settings.frameCount;
			std::string frameStats = "Frame time : "s + std::to_string( dt ) + "ms. Frame "s + std::to_string( settings.frameCount ) + "\n"s;
			console.print( frameStats );
#endif
			test();
			present();
		}
		else
		{
			// game is minimized/out-of-focus
			m_gameTimer.delayFor( 10 );
		}
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
		case VK_F11:
		{
			auto &gfx = m_mainWindow.getGraphics();
			if ( gfx.getDisplayMode() )
			{
				gfx.resize( 1600, 900 );
			}
			else
			{
				gfx.resize( 0, 0 );
			}
			break;
		}
		case VK_RETURN:
		{
			auto &gfx = m_mainWindow.getGraphics();
			gfx.getRenderer3d().dumpShadowMap( gfx, "dumps/shadow_" );
			break;
		}
		}//switch
	}

	const float camSpeed = keyboard.isKeyPressed( VK_SHIFT ) ? 6.0f : 1.0f;

	auto &activeCamera = s_cameraMan.getActiveCamera();
	if ( !m_mainWindow.isCursorEnabled() )
	{
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
			activeCamera.rotateRel( dt * camSpeed * delta->m_dx, dt * camSpeed * delta->m_dy );
		}
	}

	return 1;
}

void Sandbox3d::update( const float dt, const float lerpBetweenFrames )
{
	const auto &activeCamera = s_cameraMan.getActiveCamera();
	// binds camera to all Passes that need it
	auto &gfx = m_mainWindow.getGraphics();
	gfx.getRenderer3d().setActiveCamera( activeCamera );

	m_pPointLight1->update( gfx, dt, activeCamera.getViewMatrix() );
	//m_pPointLight2->update( gfx, dt, activeCamera.getViewMatrix() );

	m_terrain.update( dt, lerpBetweenFrames );	// #TODO: Mesh::update should actually be doing rendering and Mesh::render() simply submits bindables for rendering, so some reformatting and reorganizing here is definitely due
	m_cube1.update( dt, lerpBetweenFrames );
	m_cube2.update( dt, lerpBetweenFrames );
	m_cube3.update( dt, lerpBetweenFrames );
	//m_nanoSuit.update( dt, lerpBetweenFrames );
	m_carabiner.update( dt, lerpBetweenFrames );
	m_sponzaScene.update( dt, lerpBetweenFrames );
}

void Sandbox3d::updateFixed( const float dt )
{
	// used for stuff that need to be in-step with the physics engine, in order to guarantee a deterministic timestep that is independent of the frame-rate dt
	// apply continuous forces here (but an instantaneous impulse like a character jumping can and should be done in update instead after input detects it)
	// also animation that pertains to gameplay (or physics) should be done here (eg character animation in multiplayer game) if you care about fairness


}

void Sandbox3d::render()
{
	auto &gfx = m_mainWindow.getGraphics();
	gfx.beginFrame();

	m_pPointLight1->render( rch::lambert );
	//m_pPointLight2->render( rch::lambert );

	m_terrain.render();
	m_cube1.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_cube2.render();
	m_cube3.render();
	m_testSphere.render();
	//m_nanoSuit.render( rch::lambert | rch::shadow | rch::blurOutline );
	m_carabiner.render( rch::lambert | rch::shadow | rch::solidOutline | rch::blurOutline );
	m_sponzaScene.render( rch::lambert | rch::shadow );

	s_cameraMan.render( rch::lambert | rch::wireframe );

	gfx.runRenderer();
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
	auto &gfx = m_mainWindow.getGraphics();

	// Showcase Effect controls by passing visitors to the object hierarchies
	s_cameraMan.displayImguiWidgets( gfx );

	m_pPointLight1->displayImguiWidgets();
	//m_pPointLight2->displayImguiWidgets();

	m_terrain.displayImguiWidgets( gfx, "Terrain Base"s );

	m_cube1.displayImguiWidgets( gfx, "Cube 1"s );
	m_cube2.displayImguiWidgets( gfx, "Cube 2"s );
	m_cube3.displayImguiWidgets( gfx, "Cube 3"s );

	m_carabiner.displayImguiWidgets( gfx );

	gfx.getRenderer3d().displayImguiWidgets( gfx );

	if ( m_bShowDemoWindow )
	{
		ImGui::ShowDemoWindow( &m_bShowDemoWindow );
	}
#endif
}

void Sandbox3d::connectToRenderer( ren::Renderer3d &renderer )
{
	s_cameraMan.connectEffectsToRenderer( renderer );

	m_pPointLight1->connectEffectsToRenderer( renderer );
	//m_pPointLight2->connectEffectsToRenderer( renderer );

	m_terrain.connectEffectsToRenderer( renderer );
	m_cube1.connectEffectsToRenderer( renderer );
	m_cube2.connectEffectsToRenderer( renderer );
	m_cube3.connectEffectsToRenderer( renderer );
	m_testSphere.connectEffectsToRenderer( renderer );
	//m_nanoSuit.connectEffectsToRenderer( renderer );
	m_carabiner.connectEffectsToRenderer( renderer );
	m_sponzaScene.connectEffectsToRenderer( renderer );

	m_cube2.setEffectEnabled( rch::blurOutline, false );

	if ( m_pPointLight1->isCastingShadows() )
	{
		renderer.setShadowCamera( *m_pPointLight1->shareCamera(), true );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Arkanoid::Arkanoid( const int width,
	const int height,
	const int x,
	const int y )
	:
	Game(width, height, "Arkanoid", x, y),
	m_ball(dx::XMFLOAT2{450.0f, 450.0f}, dx::XMFLOAT2{-300.0f, -300.0f}),
	m_walls(R3ctangle(0.0f, (float)width, 0.0f, (float)height)),
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
			m_bricks[i] = Brick{R3ctangle{dx::XMFLOAT2{topLeft.x + curBrickTopLeftOffset.x, topLeft.y + curBrickTopLeftOffset.y}, s_brickWidth, s_brickHeight}, rowCol};
			++i;
		}
	}

	//static_cast<const IReporter<SwapChainResized>&>(nullptr).addListener( this );
}

//void Arkanoid::notify( const SwapChainResized &event )
//{
//	(void)event;
//	pass_;
//}

int Arkanoid::loop()
{
	int returnC0de = -1;
	m_gameTimer.start();
	while ( true )
	{
		auto exitCode = m_mainWindow.messageLoop();
		if ( exitCode )
		{
			if ( *exitCode == 0 )
			{
				return 0;
			}
		}

		const float dt = calcDt();
		returnC0de = checkInput( dt );
		if ( returnC0de == 0 )
		{
			break;
		}
		update( dt );
		render();
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		present();
	}
	return returnC0de;
}

float Arkanoid::calcDt()
{
	auto &settings = s_settingsMan.getSettings();
	float dt = m_gameTimer.lap() * settings.fGameSpeed;
	return dt;
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

void Arkanoid::render()
{
	auto &gfx = m_mainWindow.getGraphics();
	gfx.beginFrame();

	m_ball.render( gfx );
	for ( const Brick &b : m_bricks )
	{
		b.render( gfx );
	}
	m_paddle.render( gfx );

	gfx.runRenderer();
}

void Arkanoid::test()
{
#if defined _DEBUG && !defined NDEBUG
	pass_;
#endif
}