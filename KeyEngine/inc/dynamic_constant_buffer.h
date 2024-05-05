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


//=============================================================
//	\class	CBElement
//	\author	KeyC0de
//	\date	2022/08/21 19:55
//	\brief	CBElement instances form a tree that describes the layout of the data buffer
//			supporting nested aggregates of structs and arrays
//=============================================================
class CBElement final
{
	// this forms the polymorpic base for extra data that Struct and Array `CBElement`s have
	struct IExtraData
	{
		virtual ~IExtraData() = default;
	};
	// friend relationships are used liberally throught the DynamicConstant system
	// instead of seeing the various classes in this system as encapsulated decoupled units,
	// they must be viewed as aspect of one large monolithic system
	// the reason for the friend relationships is generally so that intermediate
	// classes that the client should not create can have their constructors made private,
	// so that cookLayout() cannot be called on arbitrary CBElements, etc.
	friend class RawLayout;
	friend struct ExtraData;
private:
	// each element stores its own offset. this makes lookup to find its position in the byte buffer fast.
	// Special handling is required for situations where arrays are involved
	std::optional<size_t> m_offset;
	ElementType m_type = Empty;
	std::unique_ptr<IExtraData> m_pExtraData;
public:
	// get a string signature for this element (recursive); when called on the root
	// element of a layout tree, generates a uniquely-identifying string for the layout
	// we can use that signature to store a codex of layouts and to share layouts
	std::string calcSignature() const cond_noex;
	// Check if element is "real"
	bool isValid() const noexcept;
	// calculate array indexing offset - tricky
	std::pair<size_t, const CBElement*> calculateArrayIndexingOffset( const size_t offset, const size_t index ) const cond_noex;
	// [] only works for Structs; access element (child node in tree) by name
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
	CBElement& add( const ElementType addedType, const std::string& name ) cond_noex;
	template<ElementType elem>
	CBElement& add( const std::string& key ) cond_noex
	{
		return add( elem, std::move( key ) );
	}
	// only works for Arrays; set the class and the # of elements
	CBElement& set( const ElementType addedType, const size_t size ) cond_noex;
	template<ElementType elem>
	CBElement& set( const size_t size ) cond_noex
	{
		return set( elem, size );
	}

	//	\function	fetch	||	\date	2022/08/30 0:46
	//	\brief	returns offset of leaf types for read/write purposes w/ typecheck in Debug
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
	//	\function	CBElement	||	\date	2022/08/30 0:47
	//	\brief	construct an empty layout element
	CBElement() noexcept = default;
	CBElement( const ElementType typeIn ) cond_noex;
	// sets all offsets for element and subelements, prepending padding when necessary
	// returns offset directly after this element
	const size_t commit( const size_t offsetIn ) cond_noex;
	// implementations for GetSignature for aggregate types
	std::string getSignatureForStruct() const cond_noex;
	std::string calcSignatureForArray() const cond_noex;
	// implementations for commit for aggregate types
	const size_t commitStruct( const size_t offsetIn );
	const size_t commitArray( const size_t offsetIn );

	// returns singleton instance of empty layout element
	static CBElement& getEmptyElement() noexcept
	{
		static CBElement empty{};
		return empty;
	}
	// returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
	static const size_t advanceToBoundary( const size_t offset ) noexcept;
	// return true if a memory block crosses a boundary
	static bool doesCrossBoundary( const size_t offset, const size_t size ) noexcept;
	// advance an offset to next boundary if block crosses a boundary
	static const size_t advanceIfCrossesBoundary( const size_t offset, const size_t size ) noexcept;
	// check string for validity as a struct key
	static bool validateMemberName( const std::string &name ) noexcept;
};


//=============================================================
//	\class	CBLayout
//	\author	KeyC0de
//	\date	2022/08/21 19:56
//	\brief	the layout class serves as a shell to hold the root of the CBElement tree
//			client does not create CBElements directly, create a raw layout and then
//			use it to access the elements and add on from there. When building is done,
//			raw layout is moved to BindableMap (usually via Buffer::make), and the internal layout
//			element tree is "delivered" (finalized and moved out). BindableMap returns a baked
//			layout, which the buffer can then use to initialize itself. Baked layout can
//			also be used to directly init multiple Buffers. Baked layouts are conceptually immutable.
//			Base CBLayout class cannot be constructed.
//=============================================================
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


//=============================================================
//	\class	RawLayout
//	\author	KeyC0de
//	\date	2022/08/21 19:55
//	\brief	Raw layout represents a layout that has not yet been finalized and registered
//			structure can be edited by adding layout nodes
//=============================================================
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
	// reset this object with an empty struct at its root
	void clear() noexcept;
	//	\function	cookLayout	||	\date	2022/08/21 19:50
	//	\brief	cook/commit the layout and then relinquish (by yielding the root layout element)
	std::shared_ptr<CBElement> cookLayout() noexcept;
};


//=============================================================
//	\class	CookedLayout
//	\author	KeyC0de
//	\date	2022/08/21 19:51
//	\brief	CookedLayout represents a completed and registered CBLayout shell object
//			layout tree is fixed
//=============================================================
class CookedLayout final
	: public CBLayout
{
	friend class LayoutMap;
	friend class CBuffer;
public:
	//	\function	operator[]	||	\date	2022/08/21 19:50
	//	\brief	key into the root Struct (const to disable mutation of the layout)
	const CBElement& operator[]( const std::string &key ) const cond_noex;
	//	\function	shareRootElement	||	\date	2022/08/21 19:50
	//	\brief	get a share on layout tree root
	std::shared_ptr<CBElement> shareRootElement() const noexcept;
private:
	//	\function	CookedLayout	||	\date	2022/08/21 19:50
	//	\brief	this ctor used by BindableMap to return cooked layouts
	CookedLayout( std::shared_ptr<CBElement> pRoot ) noexcept;
	//	\function	relinquishRoot	||	\date	2022/08/21 19:50
	//	\brief	use to pilfer the layout tree
	std::shared_ptr<CBElement> relinquishRoot() noexcept;
};


//=============================================================
//	\class	CBElementConstView
//	\author	KeyC0de
//	\date	2022/08/21 19:51
//	\brief	proxy class that is emitted when keying/indexing into a Buffer
//			implement conversions/assignment that allows manipulation of the raw bytes of the Buffer.
//			This version is const, only supports reading
//			Refs can be further keyed/indexed to traverse the layout structure
//=============================================================
class CBElementConstView final
{
	friend class CBuffer;
	friend class CBElementView;

	// this offset is the offset that is built up by indexing into arrays
	// accumulated for every array index in the m_path of access into the structure
	size_t m_arrayOffset;	// has to do with array indexing, it's an extra offset that accumulates as you nest inside arrays deeper into the structure - give it another name
	const CBElement *pLayout;
	const char *m_p;
private:
	// refs should only be constructable by other refs or by the buffer
	CBElementConstView( const CBElement *pLayout, const char *pBytes, size_t offset ) noexcept;
public:
	// emitted when you use addressof on the Ref
	// it allows conversion to pointer, useful for using Buffer elements with ImGui widget functions etc.
	class Ptr
	{
		friend CBElementConstView;

		const CBElementConstView *p;
	public:
		// conversion for getting read-only pointer to supported CPUType
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
	// check if the indexed element actually exists
	// this is possible because if you key into a Struct with a nonexistent key
	// it will still return an Empty CBElement that will enable this test
	// but will not enable any other kind of access
	bool isValid() const noexcept;
	// key into the current element as a struct
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


//=============================================================
//	\class	CBElementView
//	\author	KeyC0de
//	\date	2022/08/21 19:52
//	\brief	version of CBElementConstView that also allows writing to the bytes of Buffer
//			see above in CBElementConstView for detailed description
//			maintains a pointer to the byte vector of the Buffer
//=============================================================
class CBElementView final
{
	friend class CBuffer;

	// m_arrayOffset has to do with array indexing, it's an extra offset that accumulates
	// as you nest inside arrays deeper into the structure
	size_t m_arrayOffset;
	const CBElement *m_pLayout;
	char *m_p;
private:
	// refs should only be constructable by other refs or by the buffer
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
	// optionally set value if not an empty Ref
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
	// get pointer to the layout element
	Ptr operator&() const cond_noex;
	// conversion for reading/writing as a supported CPUType
	template<typename T>
	operator T&() const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in conversion");
		return *reinterpret_cast<T*>( m_p + m_arrayOffset + m_pLayout->fetch<T>() );
	}
	// assignment for writing to as a supported CPUType
	template<typename T>
	T& operator=( const T &rhs ) const cond_noex
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported CPUType used in assignment");
		return static_cast<T&>(*this) = rhs;
	}
};


//=============================================================
//	\class	LayoutMap
//	\author	KeyC0de
//	\date	2022/08/21 19:53
//	\brief	maps CBElement strings with their CBElement objects
//=============================================================
class LayoutMap
{
	static inline LayoutMap *s_pInstance;
	std::unordered_map<std::string, std::shared_ptr<con::CBElement>> m_map;
public:
	static con::CookedLayout fetch( con::RawLayout &&cbLayout ) cond_noex;
private:
	static LayoutMap& getInstance() noexcept;
};


//=============================================================
//	\class	CBuffer
//	\author	KeyC0de
//	\date	2022/08/21 19:53
//	\brief	The buffer object is a combination of a raw byte buffer with a CBElement
//			tree structure which acts as a view for those bytes
//			operator[] indexes into the root Struct, returning a Ref shell that can be
//			used to further index if struct/array, returning further Ref shells, or used
//			to access the data stored in the buffer if a Leaf element class
//=============================================================
class CBuffer final
{
	std::shared_ptr<CBElement> m_pLayoutRoot;
	std::vector<char> m_buffer;
public:
	// various resources can be used to construct a CBuffer
	CBuffer( RawLayout &&lay ) cond_noex;
	CBuffer( const CookedLayout &lay ) cond_noex;
	CBuffer( CookedLayout &&lay ) cond_noex;
	CBuffer( const CBuffer &rhs ) noexcept;
	CBuffer& operator==( const CBuffer &rhs ) noexcept;
	//===================================================
	//	\function	mctor
	//	\brief		have to be careful with this one...
	//				the buffer that has once been pilfered must not be used
	//	\date	2022/08/21 19:58
	CBuffer( CBuffer &&rhs ) noexcept;
	CBuffer& operator=( CBuffer &&rhs ) noexcept;
	~CBuffer() noexcept;

	// how you begin indexing into buffer (root is always Struct)
	CBElementView operator[]( const std::string &key ) cond_noex;
	// if Buffer is const, you only get to index into the buffer with a read-only proxy
	CBElementConstView operator[]( const std::string &key ) const cond_noex;
	const char* data() const noexcept;
	const char* getRawBytes() const noexcept;
	// size of the raw byte buffer
	size_t getSizeInBytes() const noexcept;
	const CBElement& getRootElement() const noexcept;
	// copy bytes from another buffer (layouts must match)
	void copyFrom( const CBuffer& ) cond_noex;
	// move bytes from another buffer (layouts must match)
	void moveFrom( CBuffer& ) noexcept;
	// return another sptr to the layout root
	std::shared_ptr<CBElement> shareLayoutRoot() const noexcept;
};


}// namespace con