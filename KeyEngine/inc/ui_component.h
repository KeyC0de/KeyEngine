#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <functional>
#include "key_type_traits.h"
#include "reporter_access.h"
#include "point.h"
#include "rectangle.h"


namespace DirectX
{

class SpriteBatch;
class SpriteFont;

	namespace Colors
	{

	const XMVECTORF32 White;

	}

}

class Graphics;
class Texture;
class Mesh;
class RasterizerState;

struct SwapChainResizedEvent;
struct UserPropertyChanged;
struct UiMsg;

namespace gui
{
/// \brief	UI code uses snake-case
/// \brief	non-UI code uses pascal-case

class Component
	: public IListener<SwapChainResizedEvent>,
	public IListener<UserPropertyChanged>,
	public IListener<UiMsg>
{
	using User_Property = std::pair<std::string, std::string>;	// {str_property_name, str_property_value}

public:
	enum Dock_Point	// #TODO:
	{
		Dock_Point_None,
		Top_Left,
		Top_Center,
		Top_Right,
		Center_Left,
		Center,
		Center_Right,
		Bottom_Left,
		Bottom_Center,
		Bottom_Right,
	};

	enum Text_Justification	// #TODO:
	{
		Text_Justification_Left,
		Text_Justification_Center,
		Text_Justification_Right,
	};

	enum Aspect_Ratio_Locked_Behavior	// #TODO:
	{
		Aspect_Ratio_None,
		Resize_Width_First,
		Resize_To_Biggest,
		Resize_To_Smallest,
		Resize_Width_Only,
		Resize_Height_Only,
	};
private:
	static inline unsigned s_texture_slot = 0u;
	/// \brief	set when the first Component is created, see remarks @ Component ctor
	static inline Component *s_root = nullptr;
	/// \brief	refers to components in the ui hierarchy that are top most
	static inline std::vector<Component*> s_top_most_components;
	/// \brief	world space components collision shape is always relevant to a Mesh; but they are still rendered in screen space and as such always facing the camera, eg. "character health bar" is considered world space component
	static inline std::vector<Component*> s_world_space_components;
	static inline float s_scale = 1.0f;
	static inline float s_last_dt = 0.0f;
	/// \brief	the component, excluding root, that was last hovered - updated on on_hover_off()
	static inline Component* s_last_hover = nullptr;
	/// \brief	the component, excluding root, that is currently being hovered - updated on evaluate_current_hover()
	static inline Component* s_current_hover = nullptr;
	static inline Point s_last_input_pos{0, 0};
	/// \brief	set on update() by root
	static inline float s_lerpBetweenFrames = 0.0f;
	/// \brief	set on update() by root
	static inline int s_current_tick = 0;				
	/// \brief	set on update() by root
	static inline int s_num_components = 0;
	static inline int s_num_tooltips = 0;
private:
	int m_id;
	std::string m_name;
	int m_last_update_tick = -1;
	/// \brief	Components with smaller depth numbers will render on top
	float m_depth;
	/// \brief	root component is an imaginary component and is always invisible
	bool m_is_visible = true;
	bool m_update_when_not_visible = false;
	bool m_is_topmost_object = false;
	bool m_can_move = true;
	/// \brief	a tooltip Component will be created as a child of this one
	bool m_is_tooltip = false;
	Aspect_Ratio_Locked_Behavior m_aspect_ratio_locked_behavior = Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None;
	Mesh* m_mesh = nullptr;
	/// \brief	the parent Component of the root is nullptr
	Component* m_parent;
	/// \brief	tooltips do not exist in the main tree hierarchy, but are owned and managed by the individual components they exist in
	std::unique_ptr<Component> m_tooltip;
	/// \brief	this component's children, all events are passed down the hierarchy to children
	std::vector<std::unique_ptr<Component>> m_children;
	std::vector<User_Property> m_user_properties;

	struct Component_State
	{
		RectangleI m_collision_shape;
		std::string m_name;
		std::string m_text;
		/// \brief	Interactive vs Enabled component:
		/// \brief	disabled means you can't interact with the component at all, no tooltips and no mouse interactions (for example just text or image display)
		/// \brief	interactive and disabled is to be able to show only tooltips but no clicks (so for example an material with tooltip but that you can't c lick)
		/// \brief	interactive and enabled is both tooltips and clicks (so like a button)
		bool m_is_interactive = true;
		bool m_is_enabled = false;
		bool m_can_resize_width = true;
		bool m_can_resize_height = true;
		Dock_Point m_docking = Dock_Point::Dock_Point_None;
		Text_Justification m_text_justification;
		DirectX::XMVECTORF32 m_color;
		const DirectX::XMFLOAT2 m_text_scale;
		std::shared_ptr<Texture> m_texture;
		std::function<void(Graphics&)> m_custom_render_func;

		/// \brief	image_path will either contain the path to the texture file or the flat color of the texture
		Component_State( Graphics &gfx, const RectangleI &collision_shape, const std::string &name = "default", const std::string &text = "", const std::string image_path = "", const DirectX::XMVECTORF32 &text_color = DirectX::Colors::White, const DirectX::XMFLOAT2 &text_scale = DirectX::XMFLOAT2{1.0f, 1.0f}, const bool is_interactive = true, const bool is_enabled = false, const bool can_resize_width = true, const bool can_resize_height = true, const Dock_Point docking = Dock_Point::Dock_Point_None, const Text_Justification text_justification = Text_Justification::Text_Justification_Left );

		~Component_State() noexcept;

		void update( const float dt, const Point &input_pos );
		void render( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
		void resize( const int new_width, const int new_height );
	private:
		void draw_text( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont );
		void draw_texture( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, RasterizerState *pRasterizerState );
	};

	std::vector<std::unique_ptr<Component_State>> m_states;
	int m_current_state_index = -1;	// the current state; this is what is currently displayed and handles events
public:
	/// \brief	use this function to create a Component as a child of an existing parent Component
	/// \brief	the root Component must already be constructed using the class ctor
	/// \brief	this function must have all the arguments and in the same order as the class ctor
	/// \brief	width & height will be automatically generarated for image components
	static bool create_component( Graphics &gfx, const std::string &name, const std::variant<Component*, std::string> parent, const int x, const int y, const int width, const int height, const bool is_tooltip = false, const std::vector<std::pair<std::string, std::string>> &state_texts = {}, const std::vector<std::pair<std::string, std::string>> &state_image_paths = {}, const DirectX::XMVECTORF32 &text_color = DirectX::Colors::White, const DirectX::XMFLOAT2 &text_scale = DirectX::XMFLOAT2{1.0f, 1.0f}, const std::string &tooltip_text = "", Mesh *mesh = nullptr, const bool is_interactive = true, const bool is_enabled = false, const bool visible = true, const bool update_when_not_visible = false, const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior = Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None, const Dock_Point docking = Dock_Point::Dock_Point_None, const Text_Justification text_justification = Text_Justification::Text_Justification_Left );
	static Component* find_forward( Component *comp, const std::string &name, const bool is_recursive );
	static Component* find_backward( const Component *comp, const std::string &name );
	static int get_num_components();
	static int get_num_components_including_tooltips();
	static Component* get_root();
	//static void calculate_aspect_ratio_locked_resize( const Component::Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior, const int current_width, const int current_height, int& new_width, int& new_height );
private:
	static void update_world_space_components();
	/// \brief	render ui components positioned in relevance to a mesh in 3d space
	static void render_world_space_components( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
	/// \brief	draws top most objects after rest of ui has been drawn (2d components, tooltips, WS components etc.)
	static void render_top_most_components( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
public:
	/// \brief	you must supply either a non "" text, or an <component_state_str, image_path_str> argument
	/// \brief	aside from the root all new Components should be created using the static function create_component by specifying the parent Component of the to-be-created-Component
	Component( Graphics &gfx, const std::string &name, const std::variant<Component*, std::string> parent, const int x, const int y, const int width, const int height, const bool is_tooltip = false, const std::vector<std::pair<std::string, std::string>> &state_texts = {}, const std::vector<std::pair<std::string, std::string>> &state_image_paths = {}, const DirectX::XMVECTORF32 &text_color = DirectX::Colors::White, const DirectX::XMFLOAT2 &text_scale = DirectX::XMFLOAT2{1.0f, 1.0f}, const std::string &tooltip_text = "", Mesh *mesh = nullptr, const bool is_interactive = true, const bool is_enabled = false, const bool visible = true, const bool update_when_not_visible = false, const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior = Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None, const Dock_Point docking = Dock_Point::Dock_Point_None, const Text_Justification text_justification = Text_Justification::Text_Justification_Left );
	~Component() noexcept;

	// #TODO: write copyers

	void create_tooltip( Graphics &gfx, const std::string &name, const int x, const int y, const int width, const int height, const std::vector<std::pair<std::string, std::string>> &state_texts, const DirectX::XMVECTORF32 &text_color = DirectX::Colors::White, const DirectX::XMFLOAT2 &text_scale = DirectX::XMFLOAT2{1.0f, 1.0f}, const Aspect_Ratio_Locked_Behavior aspect_ratio_locked_behavior = Aspect_Ratio_Locked_Behavior::Aspect_Ratio_None, const Dock_Point docking = Dock_Point::Dock_Point_None, const Text_Justification text_justification = Text_Justification::Text_Justification_Left );

	void notify( const SwapChainResizedEvent &event ) override;
	void notify( const UserPropertyChanged &event ) override;
	void notify( const UiMsg &event ) override;

	/// \brief	called for the root component once per tick; recursively called for entire ui hierarchy of root's children
	void update( const float dt, const Point &input_pos, const float lerpBetweenFrames );
	void render( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
	bool updated_last_tick() const noexcept;
	/// \brief	called on update and when a component is removed form the hierachy
	void evaluate_current_hover();
	void force_hover( Component *comp, const bool should_handle_hover = true );
	/// \brief	// the root Component always counts as hovered
	bool has_current_hover( const Point &input_pos ) const noexcept;
	bool is_current_hover_over_any_child() const noexcept;
	bool is_attached_to_root() const noexcept;

	void resize( const int width, const int height, const bool resize_children = true );

	/// \brief	retrieve the first found child component with given name
	Component* find( const std::string &name );

	void set_enable_top_most( const bool should_enable );
	bool is_top_most() const noexcept;
	bool is_top_most_or_child_of_top_most() const noexcept;

	/// \brief	removes the specified child from our child list
	/// \brief	returns the index of where the child was in the child list
	int divorce_child( Component &component, const bool reevaluate_current_hover = true );

	void set_user_property( const std::string &prop, const std::string &value );
	void remove_user_property( const std::string &property_name );
	bool does_property_exist( const std::string &property_name ) const noexcept;
	/// \brief	template to cover const char* and other string types
	template<typename T, typename = std::enable_if_t<is_string_v<T>>>
	const std::string& get_user_property_value( T property_name ) const
	{
		std::vector<User_Property>::const_iterator it = std::find_if( m_user_properties.begin(), m_user_properties.end(),
			[&property_name]( const User_Property &user_property ) -> bool
			{
				return user_property.first == property_name;
			} );

		if ( it != m_user_properties.end() )
		{
			return (*it).second;
		}

		return "";
	}

	void move_to( const Point &p );	// translate
	void move_relative( const Point &p );

	bool switch_to_state( const std::string &state_name );
	void set_visibility( const bool is_visible );
	void set_depth( const float depth );
	void set_can_resize_width( const bool b );
	void set_can_resize_height( const bool b );
	void set_interactivity( const bool b );
	void register_as_world_space( Mesh &mesh );

	/// \brief checks whether supplied component is a parent, or ancenstor of this
	bool is_parent_of( const Component &comp ) const noexcept;
	const Component* get_parent() const noexcept;
	Component* get_parent();
	/// \brief	retrieve a named parent component
	const Component* get_parent( const std::string &name ) const noexcept;
	Component* get_parent( const std::string &name );
	/// \brief	retrieve child with specified name
	const Component* get_child( const std::string &name ) const noexcept;
	Component* get_child( const std::string &name );
	/// \brief	retrieve child at specified index
	const Component* get_child( const unsigned index ) const noexcept;
	Component* get_child( const unsigned index );

	const std::vector<std::unique_ptr<Component>>& get_children() const noexcept;
	std::vector<std::unique_ptr<Component>>& get_children();
	unsigned get_children_count() const noexcept;

	bool can_resize_width() const noexcept;
	bool can_resize_height() const noexcept;
	bool has_text() const noexcept;
	bool has_image() const noexcept;
	bool is_tooltip() const noexcept;
	bool is_world_space() const noexcept;
	bool is_point_inside( const Point &point ) const noexcept;
	int get_id() const noexcept;
	const std::string& get_name() const noexcept;
	bool is_visible() const noexcept;
	bool is_visible_from_root() const noexcept;
	const Component_State& get_current_state() const noexcept;
	Component_State get_current_state();
	Point get_position() const noexcept;
	int get_x() const noexcept;
	int get_y() const noexcept;
	/// \brief	retrieve the width of the current state
	int get_width() const noexcept;
	/// \brief	retrieve the height of the current state
	int get_height() const noexcept;
	Point get_dimensions() const noexcept;
	Dock_Point get_docking() const noexcept;
	Text_Justification get_text_justification() const noexcept;
	bool is_interactive() const noexcept;
	bool is_enabled() const noexcept;
	bool has_even_been_updated() const noexcept;
	RectangleI get_collision_shape() const noexcept;

	void on_lmb_down( const float dt, const Point &input_pos );
	void on_rmb_down( const float dt, const Point &input_pos );

	/// \brief Callable has this signature: void(*)(Component*)
	template<typename Callable>
	void propagate_call( const Callable &f )
	{
		for( Component *child : m_children )
		{
			child->propagate_call( f );
		}
 
		f( this );
	}
private:
	void render_regular_components( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
	void render_tooltip( Graphics &gfx, DirectX::SpriteBatch *pSpriteBatch, DirectX::SpriteFont *pSpriteFont, RasterizerState *pRasterizerState );
	int get_next_id() const noexcept;
	bool validate_name( const std::string &name );
	bool can_handle_hover() const noexcept;
	bool can_handle_press() const noexcept;
	void on_hover( const float dt, const Point &input_pos );
	void on_hover_off();
};


}// namespace gui