#pragma once

#include <cassert>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "assertions_console.h"
#include "utils.h"


// master list of leaf types that generates enum elements and various switches etc.
// undefine it at the end of the file so it doesn't pollute anything else
#define CB_LEAF_TYPES \
	X( Float ) \
	X( Float2 ) \
	X( Float3 ) \
	X( Float4 ) \
	X( Matrix ) \
	X( Bool ) \
	X( Integer )


namespace con
{

namespace dx = DirectX;

enum ElementType
{
#define X( el ) el,
	CB_LEAF_TYPES
#undef X
	Struct,
	Array,
	Empty,
};

// static map of attributes of each leaf class
template<ElementType elem>
struct ElementProperties
{
	static constexpr bool valid = false;
};

template<>
struct ElementProperties<Float>
{
	using CPUType = float; // type used in the CPU side
	static constexpr size_t hlslSize = sizeof( CPUType ); // size of type on GPU/hlsl
	static constexpr const char *tag = "F1";	// used for generating the tag of the CB
	static constexpr bool valid = true; // metaprogramming flag to check validity of ElementProperties <param>
};

template<>
struct ElementProperties<Float2>
{
	using CPUType = dx::XMFLOAT2;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char *tag = "F2";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Float3>
{
	using CPUType = dx::XMFLOAT3;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char *tag = "F3";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Float4>
{
	using CPUType = dx::XMFLOAT4;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char *tag = "F4";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Matrix>
{
	using CPUType = dx::XMFLOAT4X4;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char *tag = "M4";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Bool>
{
	using CPUType = bool;
	static constexpr size_t hlslSize = 4u;
	static constexpr const char *tag = "BL";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Integer>
{
	using CPUType = int;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char *tag = "IN";
	static constexpr bool valid = true;
};

// ensures that every leaf class in master list has an entry in the static attribute map
#define X( el ) static_assert( ElementProperties<el>::valid, "Missing map implementation for " #el );
CB_LEAF_TYPES
#undef X

// enables reverse lookup from CPUType to leaf type
template<typename T>
struct ReverseMap final
{
	static constexpr bool valid = false;
};

#define X( el ) template<> \
	struct ReverseMap<typename ElementProperties<el>::CPUType> \
	{ \
		static constexpr ElementType type = el; \
		static constexpr bool valid = true; \
	};
CB_LEAF_TYPES
#undef X


///=============================================================
/// \class	CBElement
/// \author	KeyC0de
/// \date	2022/08/21 19:55
/// \brief	CBElement instances form a tree that describes the layout of the data buffer
/// \brief	supporting nested aggregates of structs and arrays
/// \brief	friend relationships are used liberally throught the DynamicConstant system
/// \brief	instead of seeing the various classes in this system as encapsulated decoupled units,
/// \brief	they must be viewed as aspect of one large monolithic system
/// \brief	the reason for the friend relationships is generally so that intermediate
/// \brief	classes that the client should not create can have their constructors made private,
/// \brief	so that cookLayout() cannot be called on arbitrary CBElements, etc.
///=============================================================
class CBElement final
{
	///=============================================================
	/// \class	IExtraData
	/// \author	KeyC0de
	/// \date	2022/08/21 13:50
	/// \brief	this forms the polymorpic base for extra data that Struct and Array `CBElement`s have
	///=============================================================
	struct IExtraData
	{
		virtual ~IExtraData() = default;
	};

	friend class RawLayout;
	friend struct ExtraData;
private:
	/// \brief	each element stores its own offset. this makes lookup to find its position in the byte buffer fast.
	/// \brief	Special handling is required for situations where arrays are involved
	std::optional<size_t> m_offset;
	ElementType m_type = Empty;
	std::unique_ptr<IExtraData> m_pExtraData;
public:
	/// \brief get a string signature for this element (recursive); when called on the root
	/// \brief element of a layout tree, generates a uniquely-identifying string for the layout
	/// \brief we can use that signature to store a codex of layouts and to share layouts
	std::string calcSignature() const cond_noex;
	/// \brief Check if element is "real"
	bool isValid() const noexcept;
	/// \brief calculate array indexing offset - tricky
	std::pair<size_t, const CBElement*> calculateArrayIndexingOffset( const size_t offset, const size_t index ) const cond_noex;
	/// \brief [] only works for Structs; access element (child node in tree) by name
	CBElement& operator[]( const std::string &key ) cond_noex;
	const CBElement& operator[]( const std::string &key ) const cond_noex;
	/// \brief T() only works for Arrays; gets the array class layout object
	/// \brief needed to further configure an array's class
	CBElement& T() cond_noex;
	const CBElement& T() const cond_noex;
	/// \brief offset based- functions only work after finalization!
	size_t getOffsetBegin() const cond_noex;
	size_t getOffsetEnd() const cond_noex;
	/// \brief	get size in bytes derived from offsets
	size_t getSizeInBytes() const cond_noex;
	/// \brief	only works for Structs; add CBElement to struct
	CBElement& add( const ElementType addedType, const std::string& name ) cond_noex;
	template<ElementType elem>
	CBElement& add( const std::string& key ) cond_noex
	{
		return add( elem, std::move( key ) );
	}
	/// \brief	only works for Arrays; set the class and the # of elements
	CBElement& set( const ElementType addedType, const size_t size ) cond_noex;
	template<ElementType elem>
	CBElement& set( const size_t size ) cond_noex
	{
		return set( elem, size );
	}

	/// \brief	returns offset of leaf types for read/write purposes w/ typecheck in Debug
	template<typename T>
	const size_t fetch() const cond_noex
	{
		using namespace std::string_literals;
		switch( m_type )
		{
#define X( el ) case el: \
	ASSERT( typeid( ElementProperties<el>::CPUType ) == typeid( T ), std::string{"Error: GPU Type "s + std::string{typeid( T ).name()} + " used with mapped Type "s + std::string{ENUM_STR( el )}}.c_str() ); \
	return *m_offset;
CB_LEAF_TYPES
#undef X
		default:
			ASSERT( false, "Tried to fetch non-leaf element" );
			return 0u;
		}
	}
private:
	/// \brief	construct an empty layout element
	CBElement() noexcept = default;
	CBElement( const ElementType typeIn ) cond_noex;
	/// \brief	sets all offsets for element and subelements, prepending padding when necessary
	/// \brief	returns offset directly after this element
	const size_t commit( const size_t offsetIn ) cond_noex;
	/// \brief	implementations for GetSignature for aggregate types
	std::string getSignatureForStruct() const cond_noex;
	std::string calcSignatureForArray() const cond_noex;
	/// \brief	implementations for commit for aggregate types
	const size_t commitStruct( const size_t offsetIn );
	const size_t commitArray( const size_t offsetIn );

	/// \brief	returns singleton instance of empty layout element
	static CBElement& getEmptyElement() noexcept
	{
		static CBElement empty{};
		return empty;
	}
	/// \brief	returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
	static const size_t advanceToBoundary( const size_t offset ) noexcept;
	/// \brief	return true if a memory block crosses a boundary
	static bool doesCrossBoundary( const size_t offset, const size_t size ) noexcept;
	/// \brief	advance an offset to next boundary if block crosses a boundary
	static const size_t advanceIfCrossesBoundary( const size_t offset, const size_t size ) noexcept;
	/// \brief	check string for validity as a struct key
	static bool validateMemberName( const std::string &name ) noexcept;
};


///=============================================================
/// \class	CBLayout
/// \author	KeyC0de
/// \date	2022/08/21 19:56
/// \brief	the layout class serves as a shell to hold the root of the CBElement tree
/// \brief	client does not create CBElements directly, create a raw layout and then
/// \brief	use it to access the elements and add on from there. When building is done,
/// \brief	raw layout is moved to BindableRegistry (usually via Buffer::make), and the internal layout
/// \brief	element tree is "delivered" (finalized and moved out). BindableRegistry returns a baked
/// \brief	layout, which the buffer can then use to initialize itself. Baked layout can
/// \brief	also be used to directly init multiple Buffers. Baked layouts are conceptually immutable.
/// \brief	Base CBLayout class cannot be constructed.
///=============================================================
class CBLayout
{
	friend class LayoutMap;
	friend class CBuffer;
protected:
	std::shared_ptr<CBElement> m_pLayoutRoot;
protected:
	CBLayout( std::shared_ptr<CBElement> pRoot ) noexcept;
public:
	size_t getSizeInBytes() const noexcept;
	std::string calcSignature() const cond_noex;
};


///=============================================================
/// \class	RawLayout
/// \author	KeyC0de
/// \date	2022/08/21 19:55
/// \brief	Raw layout represents a layout that has not yet been finalized and registered
/// \brief	structure can be edited by adding layout nodes
///=============================================================
class RawLayout final
	: public CBLayout
{
	friend class LayoutMap;
public:
	RawLayout() noexcept;

	// key into the root Struct
	CBElement& operator[]( const std::string &key ) cond_noex;
	// add an element to the root Struct
	template<ElementType elem>
	CBElement& add( const std::string &key ) cond_noex
	{
		return m_pLayoutRoot->add<elem>( key );
	}
private:
	/// \brief	reset this object with an empty struct at its root
	void clear() noexcept;
	/// \brief	cook/commit the layout and then relinquish (by yielding the root layout element)
	std::shared_ptr<CBElement> cookLayout() noexcept;
};


///=============================================================
/// \class	CookedLayout
/// \author	KeyC0de
/// \date	2022/08/21 19:51
/// \brief	CookedLayout represents a completed and registered CBLayout shell object
/// \brief	layout tree is fixed
///=============================================================
class CookedLayout final
	: public CBLayout
{
	friend class LayoutMap;
	friend class CBuffer;
public:
	/// \brief	key into the root Struct (const to disable mutation of the layout)
	const CBElement& operator[]( const std::string &key ) const cond_noex;
	/// \brief	get a share on layout tree root
	std::shared_ptr<CBElement> shareRootElement() const noexcept;
private:
	/// \brief	this ctor used by BindableRegistry to return cooked layouts
	CookedLayout( std::shared_ptr<CBElement> pRoot ) noexcept;
	/// \brief	use to pilfer the layout tree
	std::shared_ptr<CBElement> relinquishRoot() noexcept;
};


///=============================================================
/// \class	CBElementConstView
/// \author	KeyC0de
/// \date	2022/08/21 19:51
/// \brief	proxy class that is emitted when keying/indexing into a Buffer
/// \brief	implement conversions/assignment that allows manipulation of the raw bytes of the Buffer.
/// \brief	This version is const, only supports reading
/// \brief	Refs can be further keyed/indexed to traverse the layout structure
/// \brief	m_arrayOffset is the offset that is built up by indexing into arrays; it accumulates as you nest inside arrays deeper into the structure - give it another name
///=============================================================
class CBElementConstView final
{
	friend class CBuffer;
	friend class CBElementView;

	size_t m_arrayOffset;
	const CBElement *pLayout;
	const char *m_p;
private:
	// refs should only be constructable by other refs or by the buffer
	CBElementConstView( const CBElement *pLayout, const char *pBytes, size_t offset ) noexcept;
public:
	/// \brief	emitted when you use addressof on the Ref
	/// \brief	it allows conversion to pointer, useful for using Buffer elements with ImGui widget functions etc.
	class Ptr
	{
		friend CBElementConstView;

		const CBElementConstView *p;
	public:
		/// \brief	conversion for getting read-only pointer to supported CPUType
		template<typename T>
		operator const T*() const cond_noex
		{
			static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in pointer conversion" );
			return &static_cast<const T&>( *p );
		}
	private:
		Ptr( const CBElementConstView *ref ) noexcept;
	};
public:
	/// \brief check if the indexed element actually exists
	/// \brief this is possible because if you key into a Struct with a nonexistent key
	/// \brief it will still return an Empty CBElement that will enable this test, but will not enable any other kind of access
	bool isValid() const noexcept;
	/// \brief key into the current element as a struct
	CBElementConstView operator[]( const std::string &key ) const cond_noex;
	// index into the current element as an array
	CBElementConstView operator[]( size_t index ) const cond_noex;
	// emit a pointer proxy object
	Ptr operator&() const cond_noex;
	// conversion for reading as a supported CPUType
	template<typename T>
	operator const T&() const cond_noex
	{
		static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in conversion" );
		return *reinterpret_cast<const T*>( m_p + m_arrayOffset + pLayout->fetch<T>() );
	}
};


///=============================================================
/// \class	CBElementView
/// \author	KeyC0de
/// \date	2022/08/21 19:52
/// \brief	version of CBElementConstView that also allows writing to the bytes of Buffer
/// \brief	see above in CBElementConstView for detailed description
/// \brief	maintains a pointer to the byte vector of the Buffer
///=============================================================
class CBElementView final
{
	friend class CBuffer;

	size_t m_arrayOffset;	/// \brief m_arrayOffset has to do with array indexing, it's an extra offset that accumulates as you nest inside arrays deeper into the structure
	const CBElement *m_pLayout;
	char *m_p;
private:
	/// \brief	CBElementViews should only be constructable by other refs or by the buffer
	CBElementView( const CBElement *pLayout, char *pBytes, const size_t offset ) noexcept;
public:
	class Ptr
	{
		friend CBElementView;

		CBElementView *p;
	public:
		// conversion to read/write pointer to supported CPUType
		template<typename T>
		operator T*() const cond_noex
		{
			static_assert( ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in pointer conversion" );
			return &static_cast<T&>( *p );
		}
	private:
		Ptr( CBElementView *ref ) noexcept;
	};
public:
	operator CBElementConstView() const noexcept;
	bool isValid() const noexcept;
	CBElementView operator[]( const std::string &key ) const cond_noex;
	CBElementView operator[]( const size_t index ) const cond_noex;
	/// \brief	optionally set value if not an empty Ref
	template<typename S>
	bool setIfValid( const S &val ) cond_noex
	{
		if ( isValid() )
		{
			*this = val;
			return true;
		}
		return false;
	}
	/// \brief	get pointer to the layout element
	Ptr operator&() const cond_noex;
	/// \brief	conversion for reading/writing as a supported CPUType
	template<typename T>
	operator T&() const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in conversion");
		return *reinterpret_cast<T*>( m_p + m_arrayOffset + m_pLayout->fetch<T>() );
	}
	/// \brief	assignment for writing to as a supported CPUType
	template<typename T>
	T& operator=( const T &rhs ) const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in assignment");
		return static_cast<T&>(*this) = rhs;
	}
};


///=============================================================
/// \class	LayoutMap
/// \author	KeyC0de
/// \date	2022/08/21 19:53
/// \brief	maps CBElement strings with their CBElement objects
///=============================================================
class LayoutMap
{
	static inline LayoutMap *s_pInstance;
	std::unordered_map<std::string, std::shared_ptr<con::CBElement>> m_map;
public:
	static con::CookedLayout fetch( con::RawLayout &&cbLayout ) cond_noex;
private:
	static LayoutMap& getInstance() noexcept;
};


///=============================================================
/// \class	CBuffer
/// \author	KeyC0de
/// \date	2022/08/21 19:53
/// \brief	The buffer object is a combination of a raw byte buffer with a CBElement
/// \brief	tree structure which acts as a view for those bytes
/// \brief	operator[] indexes into the root Struct, returning a Ref shell that can be
/// \brief	used to further index if struct/array, returning further Ref shells, or used
/// \brief	to access the data stored in the buffer if a Leaf element class
/// \brief	various resources can be used to construct a CBuffer
///=============================================================
class CBuffer final
{
	std::shared_ptr<CBElement> m_pLayoutRoot;
	std::vector<char> m_buffer;
public:
	CBuffer( RawLayout &&lay ) cond_noex;
	CBuffer( const CookedLayout &lay ) cond_noex;
	CBuffer( CookedLayout &&lay ) cond_noex;
	CBuffer( const CBuffer &rhs ) noexcept;
	CBuffer& operator==( const CBuffer &rhs ) noexcept;
	/// \brief	have to be careful with this one...
	/// \brief	the buffer that has once been pilfered must not be used
	CBuffer( CBuffer &&rhs ) noexcept;
	CBuffer& operator=( CBuffer &&rhs ) noexcept;
	~CBuffer() noexcept;

	/// \brief	operator[] is how you begin indexing into buffer (root is always Struct)
	/// \brief	if Buffer is const, you only get to index into the buffer with a read-only proxy
	CBElementView operator[]( const std::string &key ) cond_noex;
	CBElementConstView operator[]( const std::string &key ) const cond_noex;
	const char* data() const noexcept;
	/// \brief	size of the raw byte buffer
	size_t getSizeInBytes() const noexcept;
	const CBElement& getRootElement() const noexcept;
	/// \brief	copy bytes from another buffer (layouts must match)
	void copyFrom( const CBuffer& ) cond_noex;
	/// \brief	move bytes from another buffer (layouts must match)
	void moveFrom( CBuffer& ) noexcept;
	/// \brief	return another sptr to the layout root
	std::shared_ptr<CBElement> shareLayoutRoot() const noexcept;
};


}// namespace con