#pragma once

#include <cassert>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "assertions_console.h"


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
	#define X(el) el,
	CB_LEAF_TYPES
	#undef X
	Struct,
	Array,
	Empty,
};

// static map of attributes of each leaf class
template<ElementType type>
struct ElementProperties
{
	static constexpr bool valid = false;
};

template<>
struct ElementProperties<Float>
{
	using CPUType = float; // type used in the CPU side
	static constexpr size_t hlslSize = sizeof( CPUType ); // size of type on GPU/hlsl
	static constexpr const char* tag = "F1";	// used for generating the tag of the CB
	static constexpr bool valid = true; // metaprogramming flag to check validity of ElementProperties <param>
};

template<>
struct ElementProperties<Float2>
{
	using CPUType = dx::XMFLOAT2;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char* tag = "F2";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Float3>
{
	using CPUType = dx::XMFLOAT3;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char* tag = "F3";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Float4>
{
	using CPUType = dx::XMFLOAT4;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char* tag = "F4";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Matrix>
{
	using CPUType = dx::XMFLOAT4X4;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char* tag = "M4";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Bool>
{
	using CPUType = bool;
	static constexpr size_t hlslSize = 4u;
	static constexpr const char* tag = "BL";
	static constexpr bool valid = true;
};

template<>
struct ElementProperties<Integer>
{
	using CPUType = int;
	static constexpr size_t hlslSize = sizeof( CPUType );
	static constexpr const char* tag = "IN";
	static constexpr bool valid = true;
};

// ensures that every leaf class in master list has an entry in the static attribute map
#define X( el ) static_assert( ElementProperties<el>::valid,\
	"Missing map implementation for " #el );
CB_LEAF_TYPES
#undef X

// enables reverse lookup from CPUType to leaf type
template<typename T>
struct ReverseMap final
{
	static constexpr bool valid = false;
};

#define X( el ) template<>\
struct ReverseMap<typename ElementProperties<el>::CPUType> \
{ \
	static constexpr ElementType type = el; \
	static constexpr bool valid = true; \
};
CB_LEAF_TYPES
#undef X


// CBElement instances form a tree that describes the layout of the data buffer
// supporting nested aggregates of structs and arrays
class CBElement final
{
	// this forms the polymorpic base for extra data that Struct and Array `CBElement`s have
	struct IExtraData
	{
		virtual ~IExtraData() = default;
	};
	// friend relationships are used liberally throught the DynamicConstant system
	// instead of seeing the various classes in this system as encapsulated decoupled
	// units, they must be viewed as aspect of one large monolithic system
	// the reason for the friend relationships is generally so that intermediate
	// classes that the client should not create can have their constructors made
	// private, so that commit() cannot be called on arbitrary CBElements, etc.
	friend class RawLayout;
	friend struct ExtraData;
private:
	// each element stores its own offset. this makes lookup to find its position in the byte buffer
	// fast. Special handling is required for situations where arrays are involved
	std::optional<size_t> m_offset;
	ElementType m_type = Empty;
	std::unique_ptr<IExtraData> m_pExtraData;
public:
	// get a string signature for this element (recursive); when called on the root
	// element of a layout tree, generates a uniquely-identifying string for the layout
	// we can use that signature to store a codex of layouts and to share layouts
	std::string getSignature() const cond_noex;
	// Check if element is "real"
	bool isValid() const noexcept;
	// calculate array indexing offset - tricky
	std::pair<size_t, const CBElement*> calculateArrayIndexingOffset( size_t offset,
		size_t index ) const cond_noex;
	// [] only works for Structs; access member (child node in tree) by name
	CBElement& operator[]( const std::string &key ) cond_noex;
	const CBElement& operator[]( const std::string &key ) const cond_noex;
	// T() only works for Arrays; gets the array class layout object
	// needed to further configure an array's class
	CBElement& T() cond_noex;
	const CBElement& T() const cond_noex;
	// offset based- functions only work after finalization!
	size_t getOffsetBegin() const cond_noex;
	size_t getOffsetEnd() const cond_noex;
	// get size in bytes derived from offsets
	size_t getSizeInBytes() const cond_noex;
	// only works for Structs; add CBElement to struct
	CBElement& add( ElementType addedType, std::string name ) cond_noex;
	template<ElementType typeAdded>
	CBElement& add( std::string key ) cond_noex
	{
		return add( typeAdded, std::move( key ) );
	}
	// only works for Arrays; set the class and the # of elements
	CBElement& set( ElementType addedType, size_t size ) cond_noex;
	template<ElementType typeAdded>
	CBElement& set( size_t size ) cond_noex
	{
		return set( typeAdded, size );
	}
	// returns offset of leaf types for read/write purposes w/ typecheck in Debug
	template<typename T>
	size_t fetch() const cond_noex
	{
		switch( m_type )
		{
		#define X(el) case el:\
			ASSERT( typeid( ElementProperties<el>::CPUType ) == typeid( T ),\
				"Wrong CPUType" );\
			return *m_offset;
		CB_LEAF_TYPES
		#undef X
		default:
			ASSERT( false, "Tried to fetch non-leaf element" );
			return 0u;
		}
	}
private:
	// construct an empty layout element
	CBElement() noexcept = default;
	CBElement( ElementType typeIn ) cond_noex;
	// sets all offsets for element and subelements, prepending padding when necessary
	// returns offset directly after this element
	size_t commit( size_t offsetIn ) cond_noex;
	// implementations for GetSignature for aggregate types
	std::string getSignatureForStruct() const cond_noex;
	std::string getSignatureForArray() const cond_noex;
	// implementations for commit for aggregate types
	size_t commitStruct( size_t offsetIn );
	size_t commitArray( size_t offsetIn );
	// returns singleton instance of empty layout element
	static CBElement& getEmptyElement() noexcept
	{
		static CBElement empty{};
		return empty;
	}
	// returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
	static size_t advanceToBoundary( size_t offset ) noexcept;
	// return true if a memory block crosses a boundary
	static bool doesCrossBoundary( size_t offset, size_t size ) noexcept;
	// advance an offset to next boundary if block crosses a boundary
	static size_t advanceIfCrossesBoundary( size_t offset, size_t size ) noexcept;
	// check string for validity as a struct key
	static bool validateMemberName( const std::string &name ) noexcept;
};


// the layout class serves as a shell to hold the root of the CBElement tree
// client does not create CBElements directly, create a raw layout and then
// use it to access the elements and add on from there. When building is done,
// raw layout is moved to BindableMap (usually via Buffer::make), and the internal layout
// element tree is "delivered" (finalized and moved out). BindableMap returns a baked
// layout, which the buffer can then use to initialize itself. Baked layout can
// also be used to directly init multiple Buffers. Baked layouts are conceptually
// immutable. Base CBLayout class cannot be constructed.
class CBLayout
{
	friend class LayoutMap;
	friend class Buffer;
protected:
	std::shared_ptr<CBElement> m_pLayoutRoot;
public:
	size_t getSizeInBytes() const noexcept;
	std::string getSignature() const cond_noex;
protected:
	CBLayout( std::shared_ptr<CBElement> pRoot ) noexcept;
};


// Raw layout represents a layout that has not yet been finalized and registered
// structure can be edited by adding layout nodes
class RawLayout final
	: public CBLayout
{
	friend class LayoutMap;
public:
	RawLayout() noexcept;
	// key into the root Struct
	CBElement& operator[]( const std::string &key ) cond_noex;
	// add an element to the root Struct
	template<ElementType type>
	CBElement& add( const std::string &key ) cond_noex
	{
		return m_pLayoutRoot->add<type>( key );
	}
private:
	// reset this object with an empty struct at its root
	void clear() noexcept;
	// commit the layout and then relinquish (by yielding the root layout element)
	std::shared_ptr<CBElement> commitLayout() noexcept;
};


// CookedLayout represend a completed and registered CBLayout shell object
// layout tree is fixed
class CookedLayout final
	: public CBLayout
{
	friend class LayoutMap;
	friend class Buffer;
public:
	// key into the root Struct (const to disable mutation of the layout)
	const CBElement& operator[]( const std::string &key ) const cond_noex;
	// get a share on layout tree root
	std::shared_ptr<CBElement> shareRootElement() const noexcept;
private:
	// this ctor used by BindableMap to return cooked layouts
	CookedLayout( std::shared_ptr<CBElement> pRoot ) noexcept;
	// use to pilfer the layout tree
	std::shared_ptr<CBElement> relinquishRoot() const noexcept;
};

// proxy class that is emitted when keying/indexing into a Buffer
// implement conversions/assignment that allows manipulation of the
// raw bytes of the Buffer. This version is const, only supports reading
// Refs can be further keyed/indexed to traverse the layout structure
class ConstElementView final
{
	friend class Buffer;
	friend class ElementView;

	// this offset is the offset that is built up by indexing into arrays
	// accumulated for every array index in the m_path of access into the structure
	size_t m_arrayOffset;	// has to do with array indexing, it's an extra offset that accumulates as you nest inside arrays deeper into the structure - give it another name
	const CBElement* pLayout;
	const char* m_p;
private:
	// refs should only be constructable by other refs or by the buffer
	ConstElementView( const CBElement* pLayout, const char* pBytes, size_t offset ) noexcept;
public:
	// emitted when you use addressof& on the Ref
	// it allows conversion to pointer, useful for using Buffer
	// elements with ImGui widget functions etc.
	class Ptr
	{
		friend ConstElementView;

		const ConstElementView* p;
	public:
		// conversion for getting read-only pointer to supported CPUType
		template<typename T>
		operator const T*() const cond_noex
		{
			static_assert( ReverseMap<std::remove_const_t<T>>::valid,
				"Unsupported CPUType used in pointer conversion" );
			return &static_cast<const T&>( *p );
		}
	private:
		Ptr( const ConstElementView* ref ) noexcept;
	};
public:
	// check if the indexed element actually exists
	// this is possible because if you key into a Struct with a nonexistent key
	// it will still return an Empty CBElement that will enable this test
	// but will not enable any other kind of access
	bool isValid() const noexcept;
	// key into the current element as a struct
	ConstElementView operator[]( const std::string &key ) const cond_noex;
	// index into the current element as an array
	ConstElementView operator[]( size_t index ) const cond_noex;
	// emit a pointer proxy object
	Ptr operator&() const cond_noex;
	// conversion for reading as a supported CPUType
	template<typename T>
	operator const T&() const cond_noex
	{
		static_assert( ReverseMap<std::remove_const_t<T>>::valid,
			"Unsupported CPUType used in conversion" );
		return *reinterpret_cast<const T*>( m_p + m_arrayOffset + pLayout->fetch<T>() );
	}
};

// version of ConstElementView that also allows writing to the bytes of Buffer
// see above in ConstElementView for detailed description
// maintains a pointer to the byte vector of the Buffer
class ElementView final
{
	friend class Buffer;

	// m_arrayOffset has to do with array indexing, it's an extra offset that accumulates
	// as you nest inside arrays deeper into the structure
	size_t m_arrayOffset;	
	const CBElement* m_pLayout;
	char* m_p;
private:
	// refs should only be constructable by other refs or by the buffer
	ElementView( const CBElement* pLayout, char* pBytes, size_t offset ) noexcept;
public:
	class Ptr
	{
		friend ElementView;

		ElementView* p;
	public:
		// conversion to read/write pointer to supported CPUType
		template<typename T>
		operator T*() const cond_noex
		{
			static_assert( ReverseMap<std::remove_const_t<T>>::valid,
				"Unsupported CPUType used in pointer conversion" );
			return &static_cast<T&>( *p );
		}
	private:
		Ptr( ElementView* ref ) noexcept;
	};
public:
	operator ConstElementView() const noexcept;
	bool isValid() const noexcept;
	ElementView operator[]( const std::string &key ) const cond_noex;
	ElementView operator[]( size_t index ) const cond_noex;
	// optionally set value if not an empty Ref
	template<typename S>
	bool setIfValid( const S& val ) cond_noex
	{
		if ( isValid() )
		{
			*this = val;
			return true;
		}
		return false;
	}
	// get pointer to the layout element
	Ptr operator&() const cond_noex;
	// conversion for reading/writing as a supported CPUType
	template<typename T>
	operator T&() const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid,
			"Unsupported CPUType used in conversion");
		return *reinterpret_cast<T*>( m_p + m_arrayOffset + m_pLayout->fetch<T>() );
	}
	// assignment for writing to as a supported CPUType
	template<typename T>
	T& operator=( const T& rhs ) const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid,
			"Unsupported CPUType used in assignment");
		return static_cast<T&>(*this) = rhs;
	}
};


// maps CBElement strings with their CBElement objects
class LayoutMap
{
	static inline LayoutMap* m_pInstance;
	std::unordered_map<std::string, std::shared_ptr<con::CBElement>> m_map;
public:
	static con::CookedLayout fetch( con::RawLayout&& cbLayout ) cond_noex;
	static void resetInstance();
private:
	static LayoutMap& getInstance() noexcept;
};


// The buffer object is a combination of a raw byte buffer with a CBElement
// tree structure which acts as an view/interpretation/overlay for those bytes
// operator [] indexes into the root Struct, returning a Ref shell that can be
// used to further index if struct/array, returning further Ref shells, or used
// to access the data stored in the buffer if a Leaf element class
class Buffer final
{
	std::shared_ptr<CBElement> m_pLayoutRoot;
	std::vector<char> m_buffer;
public:
	// various resources can be used to construct a Buffer
	Buffer( RawLayout&& lay ) cond_noex;
	Buffer( const CookedLayout& lay ) cond_noex;
	Buffer( CookedLayout&& lay ) cond_noex;
	~Buffer() noexcept;
	Buffer( const Buffer& rhs ) noexcept;
	Buffer& operator==( const Buffer& rhs ) noexcept;
	// have to be careful with this one...
	// the buffer that has once been pilfered must not be used :x
	Buffer( Buffer&& rhs ) noexcept;
	Buffer& operator=( Buffer&& rhs ) noexcept;

	// how you begin indexing into buffer (root is always Struct)
	ElementView operator[]( const std::string &key ) cond_noex;
	// if Buffer is const, you only get to index into the buffer with a read-only proxy
	ConstElementView operator[]( const std::string &key ) const cond_noex;
	const char* getRawBytes() const noexcept;
	// size of the raw byte buffer
	size_t getSizeInBytes() const noexcept;
	const CBElement& getRootLayoutElement() const noexcept;
	// copy bytes from another buffer (layouts must match)
	void copyFrom( const Buffer& ) cond_noex;
	// return another sptr to the layout root
	std::shared_ptr<CBElement> shareLayoutRoot() const noexcept;
};


}//::con

#ifndef CB_IMPL_SOURCE
#undef CB_LEAF_TYPES
#endif