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


// tapping into Assimp aiMesh elements given MapElementProperties
#define ASSIMP_EXTRACT_AIMESH( element ) \
	static CPUType extract( const aiMesh &mesh,\
		const size_t i ) noexcept\
	{\
		return *reinterpret_cast<const CPUType*>( &mesh.element[i] );\
	}

#define VERTEX_INPUT_LAYOUT_ELEMENT_TYPES \
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

// example VertexInputLayout: P3NT2 signifying: Position3, Normal, UV Tex Coords
// assembled by appending ILElement `tag`s next to each other
// careful the Direct3d runtime will check at runtime that there are not multiple hlslSemantics on a single Vertex Buffer Input Layout!
class VertexInputLayout final
{
public:
	// Input Layout Element
	class ILElement;
private:
	std::vector<ILElement> m_vertexLayoutElements;
public:
	enum ILEementType
	{
#define X( el ) el,
		VERTEX_INPUT_LAYOUT_ELEMENT_TYPES
#undef X
	};

	template<ILEementType>
	struct MapElementProperties;

	template<>
	struct MapElementProperties<Position2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char *hlslSemantic = "Position";
		static constexpr const char *tag = "P2";
		ASSIMP_EXTRACT_AIMESH( mVertices )
	};

	template<>
	struct MapElementProperties<Position3D>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Position";
		static constexpr const char *tag = "P3";
		ASSIMP_EXTRACT_AIMESH( mVertices )
	};

	template<>
	struct MapElementProperties<Texture2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char *hlslSemantic = "Texcoord";
		static constexpr const char *tag = "T2";
		ASSIMP_EXTRACT_AIMESH( mTextureCoords[0] )
	};

	template<>
	struct MapElementProperties<Normal>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Normal";
		static constexpr const char *tag = "N";
		ASSIMP_EXTRACT_AIMESH( mNormals )
	};

	template<>
	struct MapElementProperties<Tangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Tangent";
		static constexpr const char *tag = "Nt";
		ASSIMP_EXTRACT_AIMESH( mTangents )
	};

	template<>
	struct MapElementProperties<Bitangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Bitangent";
		static constexpr const char *tag = "Nb";
		ASSIMP_EXTRACT_AIMESH( mBitangents )
	};

	template<>
	struct MapElementProperties<Float3Color>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C3";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	template<>
	struct MapElementProperties<Float4Color>
	{
		using CPUType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C4";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	template<>
	struct MapElementProperties<BGRAColor>
	{
		using CPUType = ColorBGRA;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C9";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	//template<>
	//struct MapElementProperties<RGBAColor>
	//{
	//	using CPUType = col::RGBAColor;
	//	static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	static constexpr const char *hlslSemantic = "Color";
	//	static constexpr const char *tag = "C8";
	//	ASSIMP_EXTRACT_AIMESH( mColors[0] )
	//};

	template<>
	struct MapElementProperties<Count>
	{
		using CPUType = long double;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char *hlslSemantic = "Invalid";
		static constexpr const char *tag = "INV";
		ASSIMP_EXTRACT_AIMESH( mFaces )
	};

	//	\function	bridge	||	\date	2022/08/30 12:50
	//	\brief		use bridge() at runtime to get information about an element's properties
	//				`template<VertexInputLayout::ILEementType > class T; ` is a pimpl with an exec() method quering one of those elements
	template<template<VertexInputLayout::ILEementType> class T, typename... Args>
	static constexpr auto bridge( const VertexInputLayout::ILEementType type,
		Args&&... args ) cond_noex
	{
		switch( type )
		{
#define X( el ) case VertexInputLayout::el: \
			{ \
				return T<VertexInputLayout::el>::exec( std::forward<Args>( args )... ); \
			}
			VERTEX_INPUT_LAYOUT_ELEMENT_TYPES
#undef X
		}
		// default:
		ASSERT( false, "Invalid element type query." );
		return T<VertexInputLayout::Count>::exec( std::forward<Args>( args )... );
	}

	//=============================================================
	//	\class	ILElement
	//	\author	KeyC0de
	//	\date	2022/09/11 16:16
	//	\brief	Vertex Buffer Input Layout Element
	//=============================================================
	class ILElement final
	{
		ILEementType m_type;
		size_t m_offset;
	public:
		static constexpr size_t getTypeSize( const ILEementType type ) cond_noex;
	public:
		ILElement( const ILEementType type, const size_t offset );

		const size_t getByteOffsetAfterThisInLayout() const cond_noex;
		const size_t getByteOffsetInLayout() const;
		const ILEementType getType() const noexcept;
		const D3D11_INPUT_ELEMENT_DESC getD3dDesc() const cond_noex;
		//	\function	calcTag	||	\date	2022/08/30 12:49
		//	\brief	return a tag identifying this ILElement
		const char* calcTag() const noexcept;
	};
public:
	template<ILEementType layoutType>
	const ILElement& fetch() const cond_noex
	{
		for ( auto &e : m_vertexLayoutElements )
		{
			if ( e.getType() == layoutType )
			{
				return e;
			}
		}
		ASSERT( false, "Could not find requeted Element's type" );
		return m_vertexLayoutElements.front();
	}
	const ILElement& getElementByIndex( const size_t i ) const cond_noex;
	VertexInputLayout& add( const ILEementType type ) cond_noex;
	const size_t getSizeInBytes() const cond_noex;
	const size_t getElementCount() const noexcept;
	std::vector<D3D11_INPUT_ELEMENT_DESC> getD3DInputElementDescs() const cond_noex;
	std::string calcSignature() const cond_noex;
	bool hasType( const ILEementType& type ) const noexcept;
};//VertexInputLayout


class VBElementView final
{
	friend class VBuffer;

	char *m_p = nullptr;
	const VertexInputLayout &m_layout;
private:
	template<VertexInputLayout::ILEementType layoutType>
	struct ElementSetter final
	{
		//	\function	exec	||	\date	2022/08/30 12:49
		//	\brief	returns the D3D11 DESC of INPUT_ELEMENT_LAYOUT
		template<typename T>
		static constexpr auto exec( VBElementView *pVertex,
			char *pElement,
			T &&val ) cond_noex
		{
			return pVertex->setElement<layoutType>( pElement,
				std::forward<T>( val ) );
		}
	};
public:
	//	\function	element	||	\date	2022/08/30 12:45
	//	\brief	get reference to ILElement
	template<VertexInputLayout::ILEementType layoutType>
	auto& element() cond_noex
	{
		auto pElement = m_p + m_layout.fetch<layoutType>().getByteOffsetInLayout();
		return *reinterpret_cast<typename VertexInputLayout::MapElementProperties<layoutType>::CPUType*>( pElement );
	}
protected:
	VBElementView( char *pData, const VertexInputLayout &layout ) cond_noex;
private:
	//	\function	setElementByIndex	||	\date	2022/08/30 12:45
	//	\brief	terminal case for setElementByIndex
	template<typename T>
	void setElementByIndex( const size_t i,
		T &&val ) cond_noex
	{
		const auto &element = m_layout.getElementByIndex( i );
		auto pElement = m_p + element.getByteOffsetInLayout();
		VertexInputLayout::bridge<ElementSetter>( element.getType(),
			this,
			pElement,
			std::forward<T>( val ) );
	}
	//	\function	setElementByIndex	||	\date	2022/08/30 12:45
	//	\brief	set Vertex data by its Byte index in the buffer
	template<typename First, typename ...Rest>
	void setElementByIndex( const size_t i,
		First &&first,
		Rest&&... rest ) cond_noex
	{
		setElementByIndex( i,
			std::forward<First>( first ) );		// base case
		setElementByIndex( i + 1,
			std::forward<Rest>( rest )... );	// recursive case
	}
	//	\function	setElement	||	\date	2022/08/30 12:46
	//	\brief	helper to reduce tag duplication in setElementByIndex
	template<VertexInputLayout::ILEementType layoutType, typename SrcType>
	void setElement( char *pElement,
		SrcType &&val ) cond_noex
	{
		using Dest = typename VertexInputLayout::MapElementProperties<layoutType>::CPUType;
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

class VBElementConstView final
{
	VBElementView m_vertexView;
public:
	VBElementConstView( const VBElementView &v ) cond_noex;
	template<VertexInputLayout::ILEementType layoutType>
	const auto& getElement() const cond_noex
	{
		return const_cast<VBElementView&>( m_vertexView ).element<layoutType>();
	}
};


class VBuffer final
{
	std::vector<char> m_data;
	VertexInputLayout m_vertexLayout;
public:
	VBuffer( VertexInputLayout layout, const size_t size = 0u ) cond_noex;
	VBuffer( VertexInputLayout layout, const aiMesh &mesh );
	const char* getRawBytes() const cond_noex;
	const VertexInputLayout& getLayout() const noexcept;
	void resize( const size_t newSize ) cond_noex;
	const size_t getVertexCount() const cond_noex;
	const size_t getSizeInBytes() const cond_noex;

	template<typename ...TArgs>
	void emplaceVertex( TArgs&&... args ) cond_noex
	{
		ASSERT( sizeof...( args ) == m_vertexLayout.getElementCount(), "Parameter count doesn't match the amount of vertex Elements in the layout" );
		m_data.resize( m_data.size() + m_vertexLayout.getSizeInBytes() );
		back().setElementByIndex( 0u,
			std::forward<TArgs>( args )... );
	}
	VBElementView back() cond_noex;
	VBElementConstView back() const cond_noex;
	VBElementView front() cond_noex;
	VBElementConstView front() const cond_noex;
	VBElementView operator[]( const size_t i ) cond_noex;
	VBElementConstView operator[]( const size_t i ) const cond_noex;
};


}// namespace ver

#undef ASSIMP_EXTRACT_AIMESH
#undef VERTEX_INPUT_LAYOUT_ELEMENT_TYPES