#pragma once

#include "winner.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <type_traits>
#include <utility>
#include "color.h"
#include "assimp/scene.h"
#include "assertions_console.h"


// tapping into Assimp aiMesh members given MemberProperties
#define ASSIMP_AIMESH_EXTRACT( member ) \
	static CPUType extract( const aiMesh& mesh,\
		size_t i ) noexcept\
	{\
		return *reinterpret_cast<const CPUType*>( &mesh.member[i] );\
	}

#define VERTEX_TYPES \
	X( Position2D ) \
	X( Position3D ) \
	X( Texture2D ) \
	X( Normal ) \
	X( Tangent ) \
	X( Bitangent ) \
	X( Float3Color ) \
	X( Float4Color ) \
	X( BGRAColor ) \
	X( Count )


namespace ver
{

// example VertexLayout: P3NT2 signifying: Position3, Normal, UV Tex Coords
// assembled by appending VertexMember `tag`s next to each other
class VertexLayout final
{
public:
	class VertexMember;
private:
	std::vector<VertexMember> m_vertexMembers;
public:
	enum MemberType
	{
		#define X(el) el,
		VERTEX_TYPES
		#undef X
	};

	template<MemberType>
	struct MemberProperties;

	template<>
	struct MemberProperties<Position2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* hlslSemantic = "Position";
		static constexpr const char* tag = "P2";
		ASSIMP_AIMESH_EXTRACT( mVertices )
	};
	
	template<>
	struct MemberProperties<Position3D>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* hlslSemantic = "Position";
		static constexpr const char* tag = "P3";
		ASSIMP_AIMESH_EXTRACT( mVertices )
	};
	
	template<>
	struct MemberProperties<Texture2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* hlslSemantic = "Texcoord";
		static constexpr const char* tag = "T2";
		ASSIMP_AIMESH_EXTRACT( mTextureCoords[0] )
	};
	
	template<>
	struct MemberProperties<Normal>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* hlslSemantic = "Normal";
		static constexpr const char* tag = "N";
		ASSIMP_AIMESH_EXTRACT( mNormals )
	};
	
	template<>
	struct MemberProperties<Tangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* hlslSemantic = "Tangent";
		static constexpr const char* tag = "Nt";
		ASSIMP_AIMESH_EXTRACT( mTangents )
	};
	
	template<>
	struct MemberProperties<Bitangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* hlslSemantic = "Bitangent";
		static constexpr const char* tag = "Nb";
		ASSIMP_AIMESH_EXTRACT( mBitangents )
	};
	
	template<>
	struct MemberProperties<Float3Color>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* hlslSemantic = "Color";
		static constexpr const char* tag = "C3";
		ASSIMP_AIMESH_EXTRACT( mColors[0] )
	};
	
	template<>
	struct MemberProperties<Float4Color>
	{
		using CPUType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* hlslSemantic = "Color";
		static constexpr const char* tag = "C4";
		ASSIMP_AIMESH_EXTRACT( mColors[0] )
	};
	
	template<>
	struct MemberProperties<BGRAColor>
	{
		using CPUType = ColorBGRA;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		static constexpr const char* hlslSemantic = "Color";
		static constexpr const char* tag = "C9";
		ASSIMP_AIMESH_EXTRACT( mColors[0] )
	};

	//template<>
	//struct MemberProperties<RGBAColor>
	//{
	//	using CPUType = col::RGBAColor;
	//	static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	static constexpr const char* hlslSemantic = "Color";
	//	static constexpr const char* tag = "C8";
	//	ASSIMP_AIMESH_EXTRACT( mColors[0] )
	//};

	template<>
	struct MemberProperties<Count>
	{
		using CPUType = long double;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char* hlslSemantic = "Invalid";
		static constexpr const char* tag = "INV";
		ASSIMP_AIMESH_EXTRACT( mFaces )
	};

	// use bridge() at runtime to get information about an Element's properties member
	// `template<VertexLayout::MemberType> class T;` is a pimpl with an exec() method quering one of those members
	template<
		template<VertexLayout::MemberType> class T,
		typename... Args>
	static constexpr auto bridge( VertexLayout::MemberType type,
		Args&&... args ) cond_noex
	{
		switch( type )
		{
			#define X(el) case VertexLayout::el:\
			{\
				return T<VertexLayout::el>::exec( std::forward<Args>( args )... );\
			}
			VERTEX_TYPES
			#undef X
		}
		// default:
		ASSERT( false, "Invalid element type query." );
		return T<VertexLayout::Count>::exec( std::forward<Args>( args )... );
	}

	class VertexMember final
	{
		MemberType m_type;
		size_t m_offset;
	public:
		VertexMember( MemberType type, size_t offset );

		size_t getOffsetAfterThisInLayout() const cond_noex;
		size_t getOffsetInLayout() const;
		static constexpr size_t getTypeSize( MemberType type ) cond_noex;
		MemberType getType() const noexcept;
		D3D11_INPUT_ELEMENT_DESC getDESC() const cond_noex;
		// a tag identifying this VertexMember - taken
		const char* getTag() const noexcept;
	};
public:
	// returns VertexMember
	template<MemberType Type>
	const VertexMember& fetch() const cond_noex
	{
		for ( auto& e : m_vertexMembers )
		{
			if ( e.getType() == Type )
			{
				return e;
			}
		}
		ASSERT( false, "Could not find requeted Element's type" );
		return m_vertexMembers.front();
	}
	const VertexMember& fetchByIndex( size_t i ) const cond_noex;
	VertexLayout& add( MemberType type ) cond_noex;
	size_t getSizeInBytes() const cond_noex;
	size_t getMemberCount() const noexcept;
	std::vector<D3D11_INPUT_ELEMENT_DESC> getD3DInputElementDescs() const cond_noex;
	std::string getSignature() const cond_noex;
	bool hasType( MemberType type ) const noexcept;
};//VertexLayout


class VertexView final
{
	friend class Buffer;

	char* m_p = nullptr;
	const VertexLayout& m_layout;
private:
	template<VertexLayout::MemberType type>
	struct ElementExec final
	{
		// this generate and returns the D3D11 DESC of INPUT_ELEMENT_LAYOUT
		template<typename T>
		static constexpr auto exec( VertexView* pVertex,
			char* pElement,
			T&& val ) cond_noex
		{
			return pVertex->setMember<type>( pElement,
				std::forward<T>( val ) );
		}
	};
public:
	// get reference to VertexMember
	template<VertexLayout::MemberType Type>
	auto& getMember() cond_noex
	{
		auto pElement = m_p + m_layout.fetch<Type>().getOffsetInLayout();
		return *reinterpret_cast<typename VertexLayout::MemberProperties<Type>
			::CPUType*>( pElement );
	}
	// terminal case for setMemberByIndex
	template<typename T>
	void setMemberByIndex( size_t i,
		T&& val ) cond_noex
	{
		const auto& element = m_layout.fetchByIndex( i );
		auto pElement = m_p + element.getOffsetInLayout();
		VertexLayout::bridge<ElementExec>( element.getType(),
			this,
			pElement,
			std::forward<T>( val ) );
	}
protected:
	VertexView( char* pData, const VertexLayout& layout ) cond_noex;
private:
	// set Vertex data by its Byte index in the buffer
	template<typename First, typename ...Rest>
	void setMemberByIndex( size_t i,
		First&& first,
		Rest&&... rest ) cond_noex
	{
		setMemberByIndex( i,
			std::forward<First>( first ) );		// base case
		setMemberByIndex( i + 1,
			std::forward<Rest>( rest )... );	// recursive case
	}
	// helper to reduce tag duplication in setMemberByIndex
	template<VertexLayout::MemberType DestLayoutType, typename SrcType>
	void setMember( char* pElement,
		SrcType&& val ) cond_noex
	{
		using Dest = typename VertexLayout::MemberProperties<DestLayoutType>::CPUType;
		if constexpr( std::is_assignable<Dest, SrcType>::value )
		{
			*reinterpret_cast<Dest*>( pElement ) = val;
		}
		else
		{
			ASSERT( false, "Element type mismatch!" );
		}
	}
};

class ConstVertexView final
{
	VertexView m_vertex;
public:
	ConstVertexView( const VertexView& v ) cond_noex;
	template<VertexLayout::MemberType Type>
	const auto& element() const cond_noex
	{
		return const_cast<VertexView&>( m_vertex ).getMember<Type>();
	}
};


class Buffer final
{
	std::vector<char> m_v;
	VertexLayout m_vertexLayout;
public:
	Buffer( VertexLayout layout, size_t size = 0u ) cond_noex;
	Buffer( VertexLayout layout, const aiMesh& mesh );
	const char* getRawBytes() const cond_noex;
	const VertexLayout& getLayout() const noexcept;
	void resize( size_t newSize ) cond_noex;
	size_t getVertexCount() const cond_noex;
	size_t getSizeInBytes() const cond_noex;

	template<typename ...TArgs>
	void emplaceVertex( TArgs&&... params ) cond_noex
	{
		ASSERT( sizeof...( params ) == m_vertexLayout.getMemberCount(),
			"Parameter count doesn't match the amount of vertex Elements in the layout" );
		m_v.resize( m_v.size() + m_vertexLayout.getSizeInBytes() );
		back().setMemberByIndex( 0u,
			std::forward<TArgs>( params )... );
	}
	VertexView back() cond_noex;
	ConstVertexView back() const cond_noex;
	VertexView front() cond_noex;
	ConstVertexView front() const cond_noex;
	VertexView operator[]( size_t i ) cond_noex;
	ConstVertexView operator[]( size_t i ) const cond_noex;
};


}

#undef ASSIMP_AIMESH_EXTRACT
#ifndef VB_IMPL_SOURCE
#undef VERTEX_TYPES
#endif