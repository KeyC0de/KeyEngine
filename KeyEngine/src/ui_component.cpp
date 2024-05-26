#include "ui_component.h"
#include <unordered_map>
#include <cctype>
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include "rasterizer_state.h"
#include "graphics.h"
#include "texture.h"
#include "utils.h"
#include "reporter_listener_events.h"
#include "assertions_console.h"
#include "settings_manager.h"
#include "mesh.h"
#include "key_sound.h"
#include "d3d_utils.h"

#define m_current_state m_states[m_current_state_index]


namespace gui
{

Component::Component_State::Component_State( Graphics &gfx,
	const RectangleI &collision_shape,
	const std::string &name /*= "default"*/,
	const std::string &text /*= ""*/,
	const std::string image_path /*= ""*/,
	const DirectX::XMVECTORF32 &text_color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &text_scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/,
	const bool is_interactive /*= true*/,
	const bool is_enabled /*= false*/,
	const bool can_resize_width /*= true*/,
	const bool can_resize_height /*= true*/,
	const Dock_Point docking /*= Dock_Point::Dock_Point_None*/,
	const Text_Justification text_justification /*= Text_Justification::Text_Justification_Left*/ )
	:
	m_collision_shape{collision_shape},
	m_name{name},
	m_text{text},
	m_is_interactive{is_interactive},
	m_is_enabled{is_enabled},
	m_can_resize_width{can_resize_width},
	m_can_resize_height{can_resize_height},
	m_docking{docking},
	m_text_justification{text_justification},
	m_color{text_color},
	m_text_scale{text_scale}
{
	if ( !image_path.empty() )
	{
		auto color = util::getDirectXColorFromString( image_path );
		if ( color.has_value() )
		{
			m_color = *color;
		}
		m_texture = std::make_shared<Texture>( gfx, image_path, s_texture_slot );
		++s_texture_slot;
	}

	// some component state names carry special handling and require special handling:
	if ( name == "default_fps_counter" )
	{
		m_text = "";
		m_custom_render_func = [this] ( Graphics &gfx )
			{
				static int fpsDisplayFrameCount = 0;
				static std::string fpsText;

				++fpsDisplayFrameCount;

				auto &fpsTimer = gfx.getFpsTimer();

				// if more than 1000ms have passed count fps
				if ( fpsTimer.getDurationFromStart() > 1000 )
				{
					fpsText = std::to_string( fpsDisplayFrameCount );
#if defined _DEBUG && !defined NDEBUG
					OutputDebugStringA( fpsText.data() );
#endif
					fpsTimer.restart();
					fpsDisplayFrameCount = 0;
				}

				this->m_text = fpsText;
			};
	}
}

Component::Component_State::~Component_State() noexcept
{

}

void Component::Component_State::update( const float dt,
	const Point &input_pos )
{
	pass_;
}

void Component::Component_State::render( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	// By default SpriteBatch uses premultiplied alpha blending, no depth buffer, counter clockwise culling, and linear filtering with clamp texture addressing. You can change this by passing custom state objects to SpriteBatch::Begin. Pass null for any parameters that should use their default value.
	// SpriteBatch makes use of the following states: BlendState (Alpha Blending), Constant buffer (Vertex Shader stage, slot 0), DepthStencilState, Index buffer, Input layout, Pixel shader, Primitive topology, RasterizerState, SamplerState (Pixel Shader stage, slot 0), Shader resources (Pixel Shader stage, slot 0), Vertex buffer (slot 0), Vertex shader
	// The SpriteBatch class assumes you've already set the Render Target view, Depth Stencil view, and Viewport. It will also read the first viewport set on the device unless you've explicitly called SetViewport.
	// Be sure that if you set any of the following shaders prior to using SpriteBatch that you clear them: Geometry Shader, Hull Shader, Domain Shader, Compute Shader.

	m_custom_render_func ? m_custom_render_func( gfx ) : void(0);
	m_texture ? draw_texture( gfx, pSpriteBatch, pRasterizerState ) : void(0);
	!m_text.empty() ? draw_text( gfx, pSpriteBatch, pSpriteFont ) : void(0);
}

void Component::Component_State::resize( const int new_width,
	const int new_height )
{
	pass_;
}

void Component::Component_State::draw_text( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont )
{
	pSpriteBatch->Begin();
	pSpriteFont->DrawString( pSpriteBatch, m_text.c_str(), DirectX::XMFLOAT2{static_cast<float>( m_collision_shape.getX() ), static_cast<float>( m_collision_shape.getY() )}, m_color, 0.0f, DirectX::XMFLOAT2{0.0f, 0.0f}, m_text_scale );
	pSpriteBatch->End();
}

void Component::Component_State::draw_texture( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	RasterizerState *pRasterizerState )
{
	// #TODO: drawrectanglewithcolor?
	//you can't do that, a texture is required, just create a white texture and specify color in Draw's third parameter
	pSpriteBatch->Begin( DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, nullptr, pRasterizerState->getD3dRasterizerState().Get() );
	pSpriteBatch->Draw( m_texture->getD3dSrv().Get(), m_collision_shape );
	pSpriteBatch->End();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Component::create_component( Graphics &gfx,
	const std::string &name,
	const std::variant<Component*, std::string> parent,
	const int x,
	const int y,
	const int width,
	const int height,
	const bool is_tooltip /*= false*/,
	const std::vector<std::pair<std::string, std::string>> &state_texts /*= {}*/,
	const std::vector<std::pair<std::string, std::string>> &state_image_paths /*= {}*/,
	const DirectX::XMVECTORF32 &text_color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &text_scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/,
	const std::string &tooltip_text /*= ""*/,
	Mesh *mesh /*= nullptr*/,
	const bool is_interactive /*= true*/,
	const bool is_enabled /*= false*/,
	const bool visible /*= true*/,
	const bool update_when_not_visible /*= false*/,
	const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior /*= Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None*/,
	const Dock_Point docking /*= Dock_Point::Dock_Point_None*/,
	const Text_Justification text_justification /*= Text_Justification::Text_Justification_Left*/ )
{
	Component *p_parent = nullptr;
	if ( std::holds_alternative<Component*>( parent ) )
	{
		p_parent = std::get<Component*>( parent );
	}
	else if ( std::holds_alternative<std::string>( parent ) )
	{
		const std::string parent_name = std::get<std::string>( parent );
		p_parent = s_root->find( parent_name );
	}

	ASSERT( p_parent != nullptr, "Invalid parameters! Use Component ctor to create the root Component!" );

	// make sure the attempted-to-add-component is not already a child of parent
	if ( find_forward( std::get<Component*>( parent ), name, false ) )
	{
		return false;
	}

	p_parent->m_children.emplace_back( std::make_unique<Component>( gfx, name, parent, x, y, width, height, is_tooltip, state_texts, state_image_paths, text_color, text_scale, tooltip_text, mesh, is_interactive, is_enabled, visible, update_when_not_visible, aspect_ratio_locked_behavior, docking, text_justification ) );
	return true;
}

Component* Component::find_forward( Component *comp,
	const std::string &name,
	const bool is_recursive )
{
	if ( !comp )
	{
		return nullptr;
	}

	Component* found = nullptr;
	for ( unsigned i = 0; i < comp->get_children_count() && !found; ++i )
	{
		found = is_recursive
			? find_forward( comp->get_child( i ), name, is_recursive )
			: comp->get_child( i );
		if ( found && found->m_name == name )
		{
			break;
		}
		else
		{
			found = nullptr;
		}
	}
	return found;
}

Component* Component::find_backward( const Component *comp,
	const std::string &name )
{
	// base case, check if component is invalid
	if ( comp == nullptr )
	{
		return nullptr;
	}

	// check if the current component's parent has the specified name
	Component *found = const_cast<Component*>( comp->get_parent() );
	if ( found != nullptr && found->m_name == name )
	{
		return found;
	}

	// recursive case: recurse up the hierarchy to check grandparents
	return find_backward( found, name );
}

int Component::get_num_components()
{
	return s_num_components;
}

int Component::get_num_components_including_tooltips()
{
	return s_num_components + s_num_tooltips;
}

Component* Component::get_root()
{
	return s_root;
}

Component::Component( Graphics &gfx,
	const std::string &name,
	const std::variant<Component*, std::string> parent,
	const int x,
	const int y,
	const int width,
	const int height,
	const bool is_tooltip /*= false*/,
	const std::vector<std::pair<std::string, std::string>> &state_texts /*= {}*/,
	const std::vector<std::pair<std::string, std::string>> &state_image_paths /*= {}*/,
	const DirectX::XMVECTORF32 &text_color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &text_scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/,
	const std::string &tooltip_text /*= ""*/,
	Mesh *mesh /*= nullptr*/,
	const bool is_interactive /*= true*/,
	const bool is_enabled /*= false*/,
	const bool visible /*= true*/,
	const bool update_when_not_visible /*= false*/,
	const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior /*= Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None*/,
	const Dock_Point docking /*= Dock_Point::Dock_Point_None*/,
	const Text_Justification text_justification /*= Text_Justification::Text_Justification_Left*/ )
	:
	IListener<SwapChainResizedEvent>(),
	IListener<UserPropertyChanged>(),
	IListener<UiMsg>(),
	m_id{get_next_id()},
	m_name{name},
	m_is_tooltip{is_tooltip},
	m_is_visible{visible},
	m_update_when_not_visible{update_when_not_visible},
	m_aspect_ratio_locked_behavior{aspect_ratio_locked_behavior}
{
	if ( std::holds_alternative<Component*>( parent ) )
	{
		m_parent = std::get<Component*>( parent );
	}
	else if ( std::holds_alternative<std::string>( parent ) )
	{
		const std::string parent_name = std::get<std::string>( parent );
		m_parent = s_root->find( parent_name );
	}

	RectangleI collision_shape{x, y, width, height};

	if ( m_parent == nullptr && s_root == nullptr ) 
	{
		s_root = this;
		m_can_move = false;
		m_is_visible = false;
		m_depth = 1000.0f;
		++m_current_state_index;
		m_states.emplace_back( std::make_unique<Component_State>( gfx, collision_shape ) );
		m_current_state->m_is_interactive = false;
		m_current_state->m_can_resize_width = false;
		m_current_state->m_can_resize_height = false;
		return;	// end of root component initialization
	}

	ASSERT( is_attached_to_root(), "Component is not attached to the root! This is not allowed at this point!!!\nUse create_component static function to create a new Component as a child of an existing Component!" )

#pragma warning( disable : 6011 )
	m_depth = ( m_parent->m_depth / 2 + m_id );
#pragma warning( default : 6011 )

	ASSERT( !state_texts.empty() || !state_image_paths.empty(), "you must supply either text or an image path for the Component to properly initialize." );

	m_states.resize( state_texts.size() + state_image_paths.size() );
	for ( auto & state_name_text : state_texts )
	{
		++m_current_state_index;
		m_states[m_current_state_index] = std::make_unique<Component_State>( gfx, collision_shape, state_name_text.first, state_name_text.second, "", text_color, text_scale, is_interactive, is_enabled, true, true, docking, text_justification );
	}
	for ( auto &state_name_and_image_path : state_image_paths )
	{
		++m_current_state_index;
		m_states[m_current_state_index] = std::make_unique<Component_State>( gfx, collision_shape, state_name_and_image_path.first, "", state_name_and_image_path.second, text_color, text_scale, is_interactive, is_enabled, true, true, docking, text_justification );
	}

	if ( mesh != nullptr )
	{
		register_as_world_space( *mesh );
	}

	if ( !tooltip_text.empty() )
	{
		using namespace std::string_literals;
		std::vector<std::pair<std::string, std::string>> tooltip_comp_state_texts;
		tooltip_comp_state_texts.emplace_back( "default", tooltip_text );

		// create textual component out of tooltip
		create_tooltip( gfx, "tooltip_"s + name, x, y, width, height, tooltip_comp_state_texts, text_color, text_scale, aspect_ratio_locked_behavior, docking, text_justification );
		s_num_tooltips++;
	}

	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<const IReporter<SwapChainResizedEvent>&>( reportingNexus ).addListener( this );
	static_cast<const IReporter<UserPropertyChanged>&>( reportingNexus ).addListener( this );
	static_cast<const IReporter<UiMsg>&>( reportingNexus ).addListener( this );
}

Component::~Component()
{
	if ( this == s_root )
	{
		s_top_most_components.clear();
		s_world_space_components.clear();
		s_root = nullptr;
		s_last_hover = nullptr;
		s_current_hover = nullptr;
	}
	else
	{
		{
			auto iter = std::find( s_top_most_components.begin(), s_top_most_components.end(), this );
			if ( iter != s_top_most_components.end() )
			{
				set_enable_top_most( false );
			}
		}

		{
			auto iter = std::find( s_world_space_components.begin(), s_world_space_components.end(), this );
			if ( iter != s_world_space_components.end() )
			{
				util::removeByBackSwap( s_world_space_components, this );
			}
		}
	}

	// clear memory
	m_children.clear();
	m_user_properties.clear();
	m_states.clear();
}

void Component::create_tooltip( Graphics &gfx,
	const std::string &name,
	const int x,
	const int y,
	const int width,
	const int height,
	const std::vector<std::pair<std::string, std::string>> &state_texts,
	const DirectX::XMVECTORF32 &text_color /*= DirectX::Colors::White*/,
	const DirectX::XMFLOAT2 &text_scale /*= DirectX::XMFLOAT2{1.0f, 1.0f}*/,
	const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior /*= Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None*/,
	const Dock_Point docking /*= Dock_Point::Dock_Point_None*/,
	const Text_Justification text_justification /*= Text_Justification::Text_Justification_Left*/ )
{
	std::variant<Component*, std::string> parent;
	parent = this;
	// tooltip_text parameter is empty for a Component
	std::vector<std::pair<std::string, std::string>> compStateImages;
	m_tooltip = std::make_unique<Component>( gfx, name, parent, x, y, width, height, true, state_texts, compStateImages, text_color, text_scale, "", nullptr, true, false, false, false, aspect_ratio_locked_behavior, docking, text_justification );

	// make sure tooltip is topmost so it renders on top of everything
	m_tooltip->set_enable_top_most( true );
	m_tooltip->m_depth = 0;

	move_relative( {8, 8} );
}

void Component::notify( const SwapChainResizedEvent &event )
{
	resize( event.gfx.getClientWidth(), event.gfx.getClientHeight() );
}

void Component::notify( const UserPropertyChanged &event )
{
	(void)event;
}

void Component::notify( const UiMsg &event )
{
	(void)event;
}

void Component::update( const float dt,
	const Point &input_pos,
	const float lerpBetweenFrames )
{
	ASSERT( m_current_state, "Invalid current state!" );

	if ( this == s_root )
	{
		s_last_dt = dt;
		s_last_input_pos = input_pos;
		s_lerpBetweenFrames = lerpBetweenFrames;
		++s_current_tick;
	}

	evaluate_current_hover();

	update_world_space_components();
}

void Component::render( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	ASSERT( m_current_state, "Invalid current state!" );

	render_regular_components( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
	render_world_space_components( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
	render_top_most_components( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
}

bool Component::updated_last_tick() const noexcept
{
	return s_current_tick == m_last_update_tick + 1;
}

void Component::evaluate_current_hover()
{
	for ( auto &child : m_children )
	{
		child->evaluate_current_hover();
	}

	if ( this == s_root )
	{
		return;
	}

	if ( has_current_hover( s_last_input_pos ) )
	{
		// #FIXME: if you hover component A unhover it without hovering any other one (except root) and hover it again then you spam-enter this function
		// if you hover another component B and then you hover A again it works fine
		if ( this != s_current_hover )
		{
			// instantaneous hover stuff
			on_hover( s_last_dt, s_last_input_pos );
		}
		// continuous hover stuff
		s_current_hover = this;
		m_last_update_tick = s_current_tick;
	}
	else if ( updated_last_tick() && this == s_current_hover )
	{
		on_hover_off();
	}
}

void Component::render_regular_components( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	// if this Component is not visible (except if it's the root which is made invisible) then nothing to render
	// world_space & top_most components will be rendered separately later
	if ( ( !m_is_visible && m_parent != nullptr ) /*|| is_world_space() || is_top_most()*/ )
	{
		return;
	}

	// draw the component in its current state (text, images, ..)
	m_current_state->render( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
	
	// if we have a tooltip render it
	render_tooltip( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );

	for ( auto &child : m_children )
	{
		child->render_regular_components( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
	}
}

void Component::render_tooltip( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	if ( !m_tooltip )
	{
		return;
	}

	if ( has_current_hover( s_last_input_pos ) )
	{
		m_tooltip->set_visibility( true );
		m_tooltip->m_current_state->render( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
	}
	else
	{
		m_tooltip->set_visibility( false );
	}
}

void Component::update_world_space_components()
{
	// #TODO: transform according to its Mesh
}

void Component::render_world_space_components( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	// sort the components by depth so alpha blending works properly
	std::sort( s_world_space_components.begin(), s_world_space_components.end(),
		[]( Component *lhs, Component *rhs ) -> bool
		{
			return lhs->m_depth > rhs->m_depth;
		} );
 
	for ( Component *comp : s_world_space_components )
	{
		if ( comp->m_mesh->isRenderedThisFrame() )
		{
			if ( comp->is_visible_from_root() )
			{
				comp->get_current_state().render( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
			}
		}
	}
}

void Component::render_top_most_components( Graphics &gfx,
	DirectX::SpriteBatch *pSpriteBatch,
	DirectX::SpriteFont *pSpriteFont,
	RasterizerState *pRasterizerState )
{
	for ( Component *comp : s_top_most_components )
	{
		if ( comp->is_visible_from_root() )
		{
			comp->get_current_state().render( gfx, pSpriteBatch, pSpriteFont, pRasterizerState );
		}
	}
}

void Component::force_hover( Component *comp,
	const bool should_handle_hover /*= true*/ )
{
	s_current_hover = comp;
	if ( comp && should_handle_hover )
	{
		comp->on_hover( s_last_dt, Point(get_collision_shape().getX(), get_collision_shape().getY()) );
	}
}

bool Component::has_current_hover( const Point &input_pos ) const noexcept
{
	return can_handle_hover() ?
		is_point_inside( input_pos ) :
		false;
}

bool Component::is_current_hover_over_any_child() const noexcept
{
	if( has_current_hover( s_last_input_pos ) )
	{
		return true;
	}

	bool is_it = false;
	for ( const auto &iter : m_children )
	{
		is_it = iter->is_current_hover_over_any_child();
		if ( is_it == true )
		{
			// if it's over this child, just return, we don't care about any other children
			break;
		}

	}
	return is_it;
}

bool Component::is_attached_to_root() const noexcept
{
	return this == s_root || ( m_parent != nullptr && ( m_parent == s_root || m_parent->is_attached_to_root() ) );
}

/*
void Component::calculate_aspect_ratio_locked_resize( const Component::Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior, const int current_width, const int current_height, int& new_width, int& new_height )
{
	bool is_aspect_ratio_locked = true;
	// If aspect ratio locked then we need to maintain our current width to height ratio
	if ( !is_aspect_ratio_locked )
	{
		return;
	}
	const float width_percentage  = new_width / current_width;
	const float height_percentage = new_height / current_height;

	auto set_width_or_height = [&](bool set_width)
	{
		const float width_height_aspect_ratio = current_width / current_height;
		const float height_width_aspect_ratio = current_height / current_width;

			if (set_width)
			{
				new_width = new_height * width_height_aspect_ratio;
		}
		else
		{
			new_height = new_width * height_width_aspect_ratio;
		}
	};

	switch ( aspect_ratio_locked_behavior )
	{
		case Component::Aspect_Ratio_Locked_Behavior::Resize_Width_First:
		{
			set_width_or_height( new_height != current_height );
			break;
		}
		case Component::Aspect_Ratio_Locked_Behavior::Resize_To_Biggest:
		{
			set_width_or_height( height_percentage > width_percentage );
			break;
		}
		case Component::Aspect_Ratio_Locked_Behavior::Resize_To_Smallest:
		{
			set_width_or_height( height_percentage < width_percentage );
			break;
		}
		case Component::Aspect_Ratio_Locked_Behavior::Resize_Width_Only:
		{
			set_width_or_height( true );
			break;
		}
		case Component::Aspect_Ratio_Locked_Behavior::Resize_Height_Only:
		{
			set_width_or_height( false );
			break;
		}
		case Component::Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None:
		default:
		{
			break;
		}
	}
}
*/
#pragma warning( disable : 4244 )
void Component::resize( const int width,
	const int height,
	const bool resize_children /*= true*/ )
{
	// #TODO:
	ASSERT( m_current_state, "Invalid current state!" );

	const int previous_width = get_width();
	const int previous_height = get_height();
	const bool b_resize_width = can_resize_width();
	const bool b_resize_height = can_resize_height();
	const bool can_resize = ( width != previous_width && b_resize_width ) || ( height != previous_height &&  b_resize_height );
	if ( !can_resize )
	{
		return;
	}

	int new_width = b_resize_width ? width : previous_width;
	int new_height = b_resize_height ? height : previous_height;
	if ( b_resize_width && b_resize_height )
	{
		//calculate_aspect_ratio_locked_resize( m_aspect_ratio_locked_behavior, previous_width, previous_height, new_width, new_height );
	}

	// DO NOT MOVE THIS below resize, as if you do, scale will always be 1
	const float wScale = previous_width > 0 ? float(new_width) / float(previous_width) : 0.0f;
	const float hScale = previous_height > 0 ? float(new_height) / float(previous_height) : 0.0f;

	for ( std::vector<std::unique_ptr<Component_State>>::iterator it = m_states.begin(); it != m_states.end(); ++it )
	{
		(*it)->resize( new_width, new_height );
	}

	if ( !resize_children )
	{
		return;
	}

	for ( std::vector<std::unique_ptr<Component>>::iterator iter = m_children.begin(); iter != m_children.end(); ++iter )
	{
		Component* child = (*iter).get();

		// resize children using difference
		const float width_diff = get_width() - previous_width;
		const float height_diff = get_height() - previous_height;
		child->resize( child->get_width() + width_diff, child->get_height() + height_diff );
	}
}
#pragma warning( default : 4244 )

Component* Component::find( const std::string &name )
{
	return find_forward( this, name, true );
}

void Component::set_enable_top_most( const bool should_enable )
{
	m_is_topmost_object = should_enable;
	// even if you want to enable top most, remove it before inserting in case any of the sort criteria has changed and this is already topmost, to ensure it is put in the correct place in the topmost list (like if priority has changed)
	util::removeByBackSwap( s_top_most_components, this );
	if ( should_enable )
	{
		// the things you want to be rendered last (on top) should be to back of list since we draw them last
		s_top_most_components.push_back( this );
	}
}

bool Component::is_top_most() const noexcept
{
	return m_is_topmost_object;
}

bool Component::is_top_most_or_child_of_top_most() const noexcept
{
	if ( m_is_topmost_object )
	{
		return true;
	}

	auto iter = std::find_if( s_top_most_components.begin(), s_top_most_components.end(),
		[this]( const Component* component ) -> bool
		{
			return this == component || component->is_parent_of( *this );
		} );
	return iter != s_top_most_components.end();
}

int Component::divorce_child( Component &component,
	const bool reevaluate_current_hover /*= true */ )
{
	int index = 0;
	bool found = false;
	std::vector<std::unique_ptr<Component>>::iterator iter = m_children.begin();
	while ( !found && iter != m_children.end() )
	{
		if ( (*iter).get() == &component )
		{
			iter = m_children.erase( iter );
			component.m_parent = nullptr;
			found = true;
		}
		else
		{
			++iter;
			++index;
		}
	}

	// if the mouse was over component we need to reevaluate what the mouse is going to be over anew
	if ( reevaluate_current_hover && ( &component == s_current_hover ) )
	{
		evaluate_current_hover();
	}

	return index;
}

void Component::set_user_property( const std::string &property_name,
	const std::string &value )
{
	if ( property_name == "" )
	{
		return;
	}

	auto &reportingNexus = ReportingNexus::getInstance();

	bool is_special_case_prop = false;
	const std::string lowercase_prop{property_name};
	util::doForAll( lowercase_prop,
		[]( const char c )
		{
			return std::tolower( c );
		} );

	if ( lowercase_prop == "visibility" || lowercase_prop == "visible" )
	{
		set_visibility( value == "true" || value == "1" );
		is_special_case_prop = true;
	}
	else if ( lowercase_prop == "resizeable" || lowercase_prop == "resize" )
	{
		const bool can_resize = value == "true" || value == "1";
		set_can_resize_width( can_resize );
		set_can_resize_height( can_resize );
		is_special_case_prop = true;
	}
	else if ( lowercase_prop == "msg" )
	{
		static_cast<IReporter<UiMsg>&>( reportingNexus ).notifyListeners( UiMsg{"ui_msg"} );
	}
	else
	{
		bool found = false;
		auto it = std::find_if( m_user_properties.begin(), m_user_properties.end(),
			[&property_name] ( User_Property &prop )
			{
				return prop.first == property_name;
			} );

		if ( it != m_user_properties.end() )
		{
			it->second = value;
			found = true;
		}

		if ( !found )
		{
			m_user_properties.emplace_back( property_name, value );
		}
	}

	static_cast<IReporter<UserPropertyChanged>&>( reportingNexus ).notifyListeners( UserPropertyChanged{this, property_name} );
}

void Component::remove_user_property( const std::string &property_name )
{
	// find the target user property
	auto user_property_to_erase = std::remove_if( m_user_properties.begin(), m_user_properties.end(),
		[&property_name]( const User_Property &property )
		{
			return property_name == property.first;
		} );

	// if we found it, erase it
	if ( user_property_to_erase != m_user_properties.end() )
	{
		m_user_properties.erase(user_property_to_erase);
		auto &reportingNexus = ReportingNexus::getInstance();
		static_cast<IReporter<UserPropertyChanged>&>( reportingNexus ).notifyListeners( UserPropertyChanged{this, property_name} );
	}
}

bool Component::does_property_exist( const std::string &property_name ) const noexcept
{
	return std::any_of( m_user_properties.begin(), m_user_properties.end(),
		[&property_name]( const User_Property &property )
		{
			return property.first == property_name;
		} );
}

void Component::move_to( const Point &p )
{
	ASSERT( m_current_state, "Invalid current state!" );
	m_current_state->m_collision_shape.getX() = p.x;
	m_current_state->m_collision_shape.getY() = p.y;
}

void Component::move_relative( const Point &p )
{
	ASSERT( m_current_state, "Invalid current state!" );
	m_current_state->m_collision_shape.getX() += p.x;
	m_current_state->m_collision_shape.getY() += p.y;
}

bool Component::switch_to_state( const std::string &state_name )
{
	for ( int i = 0; i != m_states.size(); ++i )
	{
		if ( m_states[i] && m_states[i]->m_name == state_name )
		{
			m_current_state_index = i;
			return true;
		}
	}
	return false;
}

void Component::set_visibility( const bool is_visible )
{
	m_is_visible = is_visible;
}

void Component::set_depth( const float depth )
{
	m_depth = depth;
}

bool Component::is_tooltip() const noexcept
{
	return m_is_tooltip;
}

bool Component::is_world_space() const noexcept
{
	return m_mesh != nullptr;
}

bool Component::is_point_inside( const Point &point ) const noexcept
{
	const auto &pos = get_position();
	const auto &width = get_width();
	const auto &height = get_height();
	return ( point.x >= pos.x ) && ( point.x <= pos.x + width ) && ( point.y >= pos.y ) && ( point.y <= pos.y + height );
}

int Component::get_id() const noexcept
{
	return m_id;
}

const std::string& Component::get_name() const noexcept
{
	return m_name;
}

bool Component::is_visible() const noexcept
{
	return m_is_visible || this == s_root;
}

bool Component::is_visible_from_root() const noexcept
{
	return is_visible() && m_parent->is_visible();
}

const Component::Component_State& Component::get_current_state() const noexcept
{
	return *m_current_state;
}

Component::Component_State Component::get_current_state()
{
	return *m_current_state;
}

int Component::get_width() const noexcept
{	
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_collision_shape.getWidth();
}

int Component::get_height() const noexcept
{	
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_collision_shape.getHeight();
}

Point Component::get_dimensions() const noexcept
{
	return {get_width(), get_height()};
}

Component::Dock_Point Component::get_docking() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_docking;
}

Component::Text_Justification Component::get_text_justification() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_text_justification;
}

RectangleI Component::get_collision_shape() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_collision_shape;
}

bool Component::is_interactive() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_is_interactive;
}

bool Component::is_enabled() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_is_enabled;
}

bool Component::has_even_been_updated() const noexcept
{
	return m_last_update_tick == -1;
}

void Component::set_can_resize_width( const bool b )
{
	ASSERT( m_current_state, "Invalid current state!" );
	m_current_state->m_can_resize_width = b;
}

void Component::set_can_resize_height( const bool b )
{
	ASSERT( m_current_state, "Invalid current state!" );
	m_current_state->m_can_resize_height = b;
}

void Component::set_interactivity( const bool b )
{
	ASSERT( m_current_state, "Invalid current state!" );
	m_current_state->m_is_interactive = b;
}

void Component::register_as_world_space( Mesh &mesh )
{
	m_mesh = &mesh;
	s_world_space_components.push_back( this );
}

bool Component::is_parent_of( const Component &comp ) const noexcept
{
	const auto parent = comp.get_parent();
	bool found{parent && comp.get_parent() == this};
	if ( !found )
	{
		found = is_parent_of( *comp.get_parent() );
	}
	return found;
}

const Component* Component::get_parent() const noexcept
{
	return m_parent;
}

Component* Component::get_parent()
{
	return m_parent;
}

const Component* Component::get_parent( const std::string &name ) const noexcept
{
	// base case, check if this component is invalid
	if ( this == nullptr )
	{
		return nullptr;
	}

	// base case, check if this component is our target component
	if ( m_name == name )
	{
		return this;
	}

	// recursive case: recurse up the hierarchy to check parent component
	return get_parent( name );
}

Component* Component::get_parent( const std::string &name )
{
	// base case, check if this component is invalid
	if ( this == nullptr )
	{
		return nullptr;
	}

	// base case, check if this component is our target component
	if ( m_name == name )
	{
		return this;
	}

	// recursive case: recurse up the hierarchy to check parent component
	return get_parent( name );
}

const Component* Component::get_child( const std::string &name ) const noexcept
{
	const auto &iter = std::find_if( m_children.begin(), m_children.end(),
		[&name] ( const std::unique_ptr<Component> &comp )
		{
			return comp->m_name == name;
		} );
	return iter == m_children.end() ? nullptr : (*iter).get();
}

Component* Component::get_child( const std::string &name )
{
	const auto &iter = std::find_if( m_children.begin(), m_children.end(),
		[&name] ( const std::unique_ptr<Component> &comp )
		{
			return comp->m_name == name;
		} );
	return iter == m_children.end() ? nullptr : (*iter).get();
}

const Component* Component::get_child( const unsigned index ) const noexcept
{
	return m_children[index].get();
}

Component* Component::get_child( const unsigned index )
{
	return m_children[index].get();
}

const std::vector<std::unique_ptr<Component>>& Component::get_children() const noexcept
{
	return m_children;
}

std::vector<std::unique_ptr<Component>>& Component::get_children()
{
	return m_children;
}

unsigned Component::get_children_count() const noexcept
{
	return static_cast<unsigned>( m_children.size() );
}

bool Component::can_resize_width() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_can_resize_width;
}

bool Component::can_resize_height() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_can_resize_height;
}

bool Component::has_text() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return !m_current_state->m_text.empty();
}

bool Component::has_image() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_texture != nullptr;
}

Point Component::get_position() const noexcept
{
	return {get_x(), get_y()};
}

int Component::get_x() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_collision_shape.getX();
}

int Component::get_y() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );
	return m_current_state->m_collision_shape.getY();
}

int Component::get_next_id() const noexcept
{
	++s_num_components;
	return s_num_components;
}

void Component::on_lmb_down( const float dt,
	const Point &input_pos )
{
	// as the children are naturally rendered on top of the parent check whether any of the children want to handle this event before this gets a chance to do so
	for ( auto &child : m_children )
	{
		child->on_lmb_down( dt, input_pos );
	}

	if ( !can_handle_press() || !is_point_inside( input_pos ) )
	{
		return;
	}

	// do stuff
}

void Component::on_rmb_down( const float dt,
	const Point &input_pos )
{
	for ( auto &child : m_children )
	{
		child->on_rmb_down( dt, input_pos );
	}

	if ( !can_handle_press() || !is_point_inside( input_pos ) )
	{
		return;
	}

	// do stuff
}

bool Component::validate_name( const std::string &name )
{
	for ( const char c : name )
	{
		if ( c == '\\' || c == '/' || c == ':' || c == '*' || c == '\?' || c == '\"' || c == '<' || c == '>' || c == '|' )
		{
			return false;
		}
	}
	return true;
}

bool Component::can_handle_hover() const noexcept
{
	return this == s_root ||
		( is_interactive() && ( m_is_visible || m_update_when_not_visible ) && ( s_last_hover == nullptr || s_current_hover != s_last_hover ) );
}

bool Component::can_handle_press() const noexcept
{
	return this == s_root ||
		is_enabled() && ( m_is_visible || m_update_when_not_visible );
}

void Component::on_hover( const float dt,
	const Point &input_pos )
{
	// trigger a ui sound on component hover
	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<IReporter<UISoundEvent>&>( reportingNexus ).notifyListeners( UISoundEvent{UISoundEvent::Component_Hovered} );

	m_current_state->update( dt, input_pos );
}

void Component::on_hover_off()
{
	s_last_hover = s_current_hover;
	s_current_hover = nullptr;
	m_last_update_tick = 0;

	auto &reportingNexus = ReportingNexus::getInstance();
	static_cast<IReporter<UISoundEvent>&>( reportingNexus ).notifyListeners( UISoundEvent{UISoundEvent::Component_Unhovered} );
}

/*
Point Component::calc_position() const noexcept
{
	ASSERT( m_current_state, "Invalid current state!" );

	if ( !m_parent )
	{
		return g_point_zero;
	}

	switch ( m_current_state->m_docking )
	{
		const auto &parent_pos = m_parent->get_position();
		const auto &parent_width = m_parent->get_width();
		const auto &parent_height = m_parent->get_height();

		case Dock_Point::Top_Left:
			return Point{parent_pos.x, parent_pos.y};
		case Dock_Point::Top_Center:
			return Point{(parent_pos.x + parent_width / 2) - m_current_state->m_width * 0.5f, parent_pos.y};
		case Dock_Point::Top_Right:
			return Point{(parent_pos.x + parent_width) - m_current_state->m_width, parent_pos.y};
		case Dock_Point::Center_Left:
			return Point{parent_pos.x, ((parent_pos.y + m_parent->get_height() * 0.5f) - m_current_state->m_height * 0.5f)};
		case Dock_Point::Center:
			return Point{(parent_pos.x + parent_width * 0.5f) - m_current_state->m_width * 0.5f, ((parent_pos.y + m_parent->get_height() * 0.5f) - m_current_state->m_height * 0.5f)};
		case Dock_Point::Center_Right:
			return Point{(parent_pos.x + parent_width) - m_current_state->m_width, ((parent_pos.y + m_parent->get_height() * 0.5f) - m_current_state->m_height * 0.5f)};
		case Dock_Point::Bottom_Left:
			return Point{parent_pos.x, (parent_pos.y + m_parent->get_height()) - m_current_state->m_height};
		case Dock_Point::Bottom_Center:
			return Point{(parent_pos.x + parent_width * 0.5f) - m_current_state->m_width * 0.5f, (parent_pos.y + m_parent->get_height()) - m_current_state->m_height};
		case Dock_Point::Bottom_Right:
			return Point{(parent_pos.x + parent_width) - m_current_state->m_width, (parent_pos.y + m_parent->get_height()) - m_current_state->m_height};
	}

	return {-1, -1};
}
*/


}// namespace gui