#include "game.h"
#include <algorithm>
#include "reporter_access.h"
#include "game_state.h"
#include "camera.h"
#include "light_source.h"
#include "rendering_channel.h"
#include "console.h"
#include "assertions_console.h"
#include "mesh.h"
#include "graphics.h"
#include "graphics_mode.h"
#include "os_utils.h"
#include "color.h"
#include "..\resource.h"
#include "renderer.h"
#include "ui_pass.h"
#include "ui_component.h"
#include "cube.h"
#include "sphere.h"
#include "line.h"
#include "plane.h"
#include "global_constants.h"
#ifndef FINAL_RELEASE
#	include "imgui/imgui.h"
#	include "imgui_visitors.h"
#endif
#if defined _DEBUG && !defined NDEBUG
#	include "bindable_registry.h"
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
	m_pImguiMan{createImguiManager()},
#endif
	m_mainWindow{width, height, title.c_str(), MAIN_WINDOW_CLASS_NAME, Window::windowProc, x, y, ColorBGRA{255, 255, 255}, LoadMenuW( THIS_INSTANCE, MAKEINTRESOURCEW( IDR_MENU_APP ) )},
	m_pCurrentState{std::make_unique<GameState>()}
{
	s_nWindows = nWindows;
	m_gameTimer.start();
}

#ifndef FINAL_RELEASE
template <typename T>
ImguiManager* Game<T>::createImguiManager() noexcept
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
#endif

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
void Game<T>::setState( std::unique_ptr<IState> pNewState,
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
const IState* Game<T>::getState() const noexcept
{
	return m_pCurrentState.get();
}

template <typename T>
IState* Game<T>::getState() noexcept
{
	return m_pCurrentState.get();
}

template<typename T>
void Game<T>::present( Graphics &gfx )
{
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
	IListener<SwapChainResizedEvent>()
{
	const auto &settings = s_settingsMan.getSettings();

	auto &gfx = m_mainWindow.getGraphics();

	s_cameraMan.add( std::make_shared<Camera>( gfx, gfx.getClientWidth(), gfx.getClientHeight(), 60.0f, dx::XMFLOAT3{0.0f, 0.0f, 0.0f}, 0.0f, 90.0f, false, true, 0.5f, 1000.0f ) );
	s_cameraMan.add( std::make_shared<Camera>( gfx, gfx.getClientWidth(), gfx.getClientHeight(), 45.0f, dx::XMFLOAT3{-13.5f, 28.8f, -6.4f}, 13.0f, 61.0f, false, true, 0.5f, 400.0f ) );


	m_lights.reserve( settings.iMaxShadowCastingDynamicLights );
	//m_lights.push_back( std::make_unique<DirectionalLight>( gfx, 0.5f, DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3{6.0f, 6.0f, -1.0f}, DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, true, true, 1.0f ) );
	m_lights.push_back( std::make_unique<PointLight>( gfx, 0.5f, DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, DirectX::XMFLOAT3{10.0f, 5.0f, -1.4f}, DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f}, true, true, 1.0f ) );
	m_lights.push_back( std::make_unique<PointLight>( gfx, 1.0f, DirectX::XMFLOAT3{0.0f, 1.0f, 0.f}, DirectX::XMFLOAT3{5.0f, 15.0f, 0.0f}, DirectX::XMFLOAT4{1.0f, 1.0f, 0.2f, 0.25f}, true, true, 0.5f ) );


	m_models.reserve( 16 );

	m_models.emplace_back( std::make_unique<Line>(gfx, 4.0f, DirectX::XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f}), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{10.0f, 10.0f, 6.0f} );
	m_models.emplace_back( std::make_unique<Cube>(gfx, 4.0f, "assets/models/brick_wall/brick_wall_diffuse.jpg"), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{10.0f, 5.0f, 6.0f} );
	m_models.emplace_back( std::make_unique<Cube>(gfx, 1.0f, "assets/models/brick_wall/brick_wall_diffuse.jpg"), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{9.9f, 4.9f, 1.4f} );
	m_models.emplace_back( std::make_unique<Cube>(gfx, 1.0f, DirectX::XMFLOAT4{1.0f, 0.6f, 1.0f, 0.6f}), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{22.0f, 12.0f, 14.0f} );
	m_models.emplace_back( std::make_unique<Sphere>(gfx, 1.0f, DirectX::XMFLOAT4{1.0f, 1.0f, 0.0f, 1.0f}), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{40.0f, 20.0f, 8.0f} );
	m_models.emplace_back( std::make_unique<Plane>(gfx, 1.0f, DirectX::XMFLOAT4{1.0f, 0.0f, 0.1f, 0.8f}, 8, 8), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{40.0f, 20.0f, 20.0f} );
	m_models.emplace_back( std::make_unique<Plane>(gfx, 1.0f, DirectX::XMFLOAT4{0.1f, 1.0f, 0.0f, 0.5f}, 4, 4), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{40.0f, 20.0f, 16.0f} );
	m_models.emplace_back( std::make_unique<Plane>(gfx, 1.0f, "assets/models/brick_wall/brick_wall_diffuse.jpg", 6, 6), gfx, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{40.0f, 20.0f, 12.0f} );
	m_models.emplace_back( gfx, "assets/models/carabiner/carabiner_hook.fbx", 1.0f, dx::XMFLOAT3{50, 0, 0}, dx::XMFLOAT3{-10.0f, 3.0f, 0.0f}/*{0.0f, 0.0f, 0.0f}*/ );
	m_models.emplace_back( gfx, "assets/models/sponza/sponza.obj", 1.0f / 8.0f, dx::XMFLOAT3{0, 0, 0}, dx::XMFLOAT3{0.0f, 0.0f, 0.0f} );

	connectToRenderer( gfx.getRenderer3d() );

	m_gui = std::make_unique<gui::UIPass>( gfx );

	auto menuState = std::make_unique<MenuState>();
	setState( std::move( menuState ), m_mainWindow.getMouse() );

	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<const IReporter<SwapChainResizedEvent>&>( reportingNexus ).addListener( this );
}

Sandbox3d::~Sandbox3d() noexcept = default;

void Sandbox3d::notify( const SwapChainResizedEvent &event )
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

	Graphics &gfx = m_mainWindow.getGraphics();

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
			returnC0de = processInput( dt );
			if ( returnC0de == 0 )
			{
				break;
			}
			float lerpBetweenFrames = runFixedLoop( dt );
			update( gfx, dt, lerpBetweenFrames );
			render( gfx );

#ifndef FINAL_RELEASE
			using namespace std::string_literals;
			KeyConsole &console = KeyConsole::getInstance();
			++settings.frameCount;
			std::string frameStats = "Frame time : "s + std::to_string( dt ) + "ms. Frame "s + std::to_string( settings.frameCount ) + "\n"s;
			console.print( frameStats );

			test( gfx );
#endif
			present( gfx );
		}
		else
		{
			// game is minimized/out-of-focus
			SleepTimer::sleepFor( 10 );
		}
	}

	return returnC0de;
}

int Sandbox3d::processInput( const float dt )
{
	auto &keyboard = m_mainWindow.getKeyboard();
	auto &mouse = m_mainWindow.getMouse();

	const bool bCursorEnabled = m_mainWindow.isCursorEnabled();

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
			if ( bCursorEnabled )	// #TODO: check the game state on how to respond not whether the cursor is enabled
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
	if ( bCursorEnabled )
	{
		while ( const auto ev = mouse.readEventQueue() )
		{
			gui::Point ui_point{mouse.getX(), mouse.getY()};
			if ( ev->isLmbPressed() )
			{
				// #FIXME: once lmb is down it doesn't go up
				m_gui->getRoot()->on_lmb_down( dt, ui_point );
			}
			else if ( ev->isRmbPressed() )
			{
				m_gui->getRoot()->on_rmb_down( dt, ui_point );
			}
		}
	}
	else
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

	// rotate Camera if in game mode
	while ( const auto &delta = mouse.readRawDeltaBuffer() )
	{
		if ( !m_mainWindow.isCursorEnabled() )
		{
			activeCamera.rotateRel( dt * camSpeed * delta->m_dx, dt * camSpeed * delta->m_dy );
		}
	}

	return 1;
}

void Sandbox3d::update( Graphics &gfx,
	const float dt,
	const float lerpBetweenFrames )
{
	static const auto &settings = s_settingsMan.getSettings();

	const auto &activeCamera = s_cameraMan.getActiveCamera();
	activeCamera.makeActive( gfx );
	gfx.getRenderer3d().setActiveCamera( activeCamera );

	//if ( util::modulus( gfx.getFrameNum(), g_nFramesPerShadowUpdate ) == 0 )	// #OPTIMIZATION: no need to update shadows every frame
	{
		std::sort( m_lights.begin(), m_lights.end(), [] (std::unique_ptr<ILightSource> &lhs, std::unique_ptr<ILightSource> &rhs)
			{
				// shadow-casting lights come first
				const unsigned lhsShadowCasting = lhs->isCastingShadows() ? 0 : 1;
				const unsigned rhsShadowCasting = rhs->isCastingShadows() ? 0 : 1;
				if ( lhsShadowCasting != rhsShadowCasting )
				{
					return lhsShadowCasting < rhsShadowCasting;
				}

				// non-frustum culled lights come second
				const unsigned lhsFrustumCulled = lhs->isFrustumCulled() ? 1 : 0;
				const unsigned rhsFrustumCulled = rhs->isFrustumCulled() ? 1 : 0;
				if ( lhsFrustumCulled != rhsFrustumCulled )
				{
					return lhsFrustumCulled < rhsFrustumCulled;
				}

				// ordering by type of light: 1. Directional Lights, 2. Spot-lights, 3. Point lights
				const unsigned lhsLightTypeId = (unsigned) lhs->getType();
				const unsigned rhsLightTypeId = (unsigned) rhs->getType();
				if ( lhsLightTypeId != rhsLightTypeId )
				{
					return lhsLightTypeId < rhsLightTypeId;
				}

				return false;
			} );
	}

	s_cameraMan.update( dt, lerpBetweenFrames, settings.bEnableSmoothMovement );

	for ( auto &pLight : m_lights )
	{
		pLight->update( dt, lerpBetweenFrames, settings.bEnableSmoothMovement );
	}

	m_terrain.update( dt, lerpBetweenFrames, settings.bEnableSmoothMovement );

	for ( auto &model : m_models )
	{
		model.update( dt, lerpBetweenFrames, settings.bEnableSmoothMovement );
	}

	auto &mouse = m_mainWindow.getMouse();
	gui::Point ui_point{mouse.getX(), mouse.getY()};
	m_gui->update( dt, ui_point, lerpBetweenFrames );
}

void Sandbox3d::updateFixed( const float dt )
{

}

void Sandbox3d::render( Graphics &gfx )
{
	gfx.beginFrame();

	s_cameraMan.render( rch::opaque | rch::wireframe );

	for ( auto &pLight : m_lights )
	{
		pLight->render( rch::opaque );
	}

	m_terrain.render( rch::opaque | rch::wireframe );
	for ( auto &model : m_models )
	{
		model.render();
	}

	//if ( util::modulus( gfx.getFrameNum(), g_nFramesPerShadowUpdate ) == 1 )	// #OPTIMIZATION: no need to update shadows every frame
	{
		static const auto &settings = s_settingsMan.getSettings();

		std::vector<ILightSource*> shadowCastingUnculledLights;
		shadowCastingUnculledLights.reserve( settings.iMaxShadowCastingDynamicLights );
		for ( const auto &pLight : m_lights )
		{
			if ( pLight->isCastingShadows() && !pLight->isFrustumCulled() )
			{
				shadowCastingUnculledLights.push_back( pLight.get() );
			}
		}

		gfx.getRenderer3d().bindShadowCastingLights( gfx, shadowCastingUnculledLights );
	}

	gfx.runRenderer();

	m_gui->render( gfx );
}

void Sandbox3d::test( Graphics &gfx )
{
#ifndef FINAL_RELEASE
	using namespace std::string_literals;
	//KeyConsole &console = KeyConsole::getInstance();

	//const BindableRegistry &instanceToBeInspected = BindableRegistry::getInstance();
	//console.print( "BindableRegistry instance count: "s + std::to_string( BindableRegistry::getInstanceCount() ) + "\n"s );
	//console.print( "BindableRegistry garbage count: "s + std::to_string( BindableRegistry::getGarbageCount() ) + "\n"s );


	//const auto &carabiner =  m_models.back();
	//if (s1.find(s2) != std::string::npos) {
	//	std::cout << "found!" << '\n';
	//}
	//if ( carabiner.getName() ==  )
	//console.print( "Current distance from carabiner: "s + std::to_string(  m_carabiner.getDistanceFromActiveCamera() ) + "\n"s );

	/// Render Imgui stuff

	// showcase Material controls by passing visitors to the object hierarchies
	s_cameraMan.displayImguiWidgets( gfx );

	for ( auto &light : m_lights )
	{
		light->displayImguiWidgets();
	}

	m_terrain.displayImguiWidgets( gfx );

	for ( auto &model : m_models )
	{
		model.displayImguiWidgets( gfx );
	}

	gfx.getRenderer3d().displayImguiWidgets( gfx );

	if ( m_bShowDemoWindow )
	{
		ImGui::ShowDemoWindow( &m_bShowDemoWindow );
	}
#endif
}

void Sandbox3d::connectToRenderer( ren::Renderer3d &renderer )
{
	s_cameraMan.connectMaterialsToRenderer( renderer );
	for ( auto &pLight : m_lights )
	{
		pLight->connectMaterialsToRenderer( renderer );
	}

	m_terrain.connectMaterialsToRenderer( renderer );
	m_terrain.setMaterialEnabled( rch::opaque, false );
	for ( auto &model : m_models )
	{
		model.connectMaterialsToRenderer( renderer );
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
	m_walls(RectangleF(0.0f, (float)width, 0.0f, (float)height)),
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
			m_bricks[i] = Brick{RectangleF{dx::XMFLOAT2{topLeft.x + curBrickTopLeftOffset.x, topLeft.y + curBrickTopLeftOffset.y}, s_brickWidth, s_brickHeight}, rowCol};
			++i;
		}
	}
}

int Arkanoid::loop()
{
	int returnC0de = -1;
	Graphics &gfx = m_mainWindow.getGraphics();

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
		returnC0de = processInput( dt );
		if ( returnC0de == 0 )
		{
			break;
		}
		update( gfx, dt );
		render( gfx );
#if defined _DEBUG && !defined NDEBUG
		test();
#endif
		present( gfx );
	}
	return returnC0de;
}

float Arkanoid::calcDt()
{
	const auto &settings = s_settingsMan.getSettings();
	float dt = m_gameTimer.lap() * settings.fGameSpeed;
	return dt;
}

int Arkanoid::processInput( const float dt )
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
		m_paddle.setTranslationRel( -s_speed * dt );
	}
	if ( keyboard.isKeyPressed( VK_RIGHT ) )
	{
		m_paddle.setTranslationRel( s_speed * dt );
	}
	if ( keyboard.isKeyPressed( VK_BACK ) )
	{
		return 0;
	}
	return 1;
}

void Arkanoid::update( Graphics &gfx,
	const float dt )
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

void Arkanoid::render( Graphics &gfx )
{
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