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


// tapping into Assimp aiMesh elements given VertexInputLayoutElementProperties
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
// assembled by appending VertexInputLayoutElement `tag`s next to each other
// careful the Direct3d runtime will check at runtime that there are not multiple hlslSemantics on a single Vertex Buffer Input Layout!
class VertexInputLayout final
{
public:
	class VertexInputLayoutElement;
private:
	std::vector<VertexInputLayoutElement> m_vertexLayoutElements;
public:
	enum VertexInputLayoutElementType
	{
		#define X(el) el,
		VERTEX_INPUT_LAYOUT_ELEMENT_TYPES
		#undef X
	};

	template<VertexInputLayoutElementType>
	struct VertexInputLayoutElementProperties;

	template<>
	struct VertexInputLayoutElementProperties<Position2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char *hlslSemantic = "Position";
		static constexpr const char *tag = "P2";
		ASSIMP_EXTRACT_AIMESH( mVertices )
	};

	template<>
	struct VertexInputLayoutElementProperties<Position3D>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Position";
		static constexpr const char *tag = "P3";
		ASSIMP_EXTRACT_AIMESH( mVertices )
	};

	template<>
	struct VertexInputLayoutElementProperties<Texture2D>
	{
		using CPUType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char *hlslSemantic = "Texcoord";
		static constexpr const char *tag = "T2";
		ASSIMP_EXTRACT_AIMESH( mTextureCoords[0] )
	};

	template<>
	struct VertexInputLayoutElementProperties<Normal>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Normal";
		static constexpr const char *tag = "N";
		ASSIMP_EXTRACT_AIMESH( mNormals )
	};

	template<>
	struct VertexInputLayoutElementProperties<Tangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Tangent";
		static constexpr const char *tag = "Nt";
		ASSIMP_EXTRACT_AIMESH( mTangents )
	};

	template<>
	struct VertexInputLayoutElementProperties<Bitangent>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Bitangent";
		static constexpr const char *tag = "Nb";
		ASSIMP_EXTRACT_AIMESH( mBitangents )
	};

	template<>
	struct VertexInputLayoutElementProperties<Float3Color>
	{
		using CPUType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C3";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	template<>
	struct VertexInputLayoutElementProperties<Float4Color>
	{
		using CPUType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C4";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	template<>
	struct VertexInputLayoutElementProperties<BGRAColor>
	{
		using CPUType = ColorBGRA;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		static constexpr const char *hlslSemantic = "Color";
		static constexpr const char *tag = "C9";
		ASSIMP_EXTRACT_AIMESH( mColors[0] )
	};

	//template<>
	//struct VertexInputLayoutElementProperties<RGBAColor>
	//{
	//	using CPUType = col::RGBAColor;
	//	static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//	static constexpr const char *hlslSemantic = "Color";
	//	static constexpr const char *tag = "C8";
	//	ASSIMP_EXTRACT_AIMESH( mColors[0] )
	//};

	template<>
	struct VertexInputLayoutElementProperties<Count>
	{
		using CPUType = long double;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char *hlslSemantic = "Invalid";
		static constexpr const char *tag = "INV";
		ASSIMP_EXTRACT_AIMESH( mFaces )
	};

	//===================================================
	//	\function	bridge
	//	\brief  	use bridge() at runtime to get information about a element's properties
	//				`template<VertexInputLayout::VertexInputLayoutElementType > class T; ` is a pimpl with an exec() method quering one of those elements
	//	\date	2022/08/30 12:50
	template<template<VertexInputLayout::VertexInputLayoutElementType> class T, typename... Args>
	static constexpr auto bridge( const VertexInputLayout::VertexInputLayoutElementType type,
		Args&&... args ) cond_noex
	{
		switch( type )
		{
			#define X(el) case VertexInputLayout::el:\
			{\
				return T<VertexInputLayout::el>::exec( std::forward<Args>( args )... );\
			}
			VERTEX_INPUT_LAYOUT_ELEMENT_TYPES
			#undef X
		}
		// default:
		ASSERT( false, "Invalid element type query." );
		return T<VertexInputLayout::Count>::exec( std::forward<Args>( args )... );
	}

	class VertexInputLayoutElement final
	{
		VertexInputLayoutElementType m_type;
		size_t m_offset;
	public:
		static constexpr size_t getTypeSize( const VertexInputLayoutElementType type ) cond_noex;
	public:
		VertexInputLayoutElement( const VertexInputLayoutElementType type, const size_t offset );

		const size_t getByteOffsetAfterThisInLayout() const cond_noex;
		const size_t getByteOffsetInLayout() const;
		const VertexInputLayoutElementType getType() const noexcept;
		const D3D11_INPUT_ELEMENT_DESC getD3dDesc() const cond_noex;
		//===================================================
		//	\function	calcTag
		//	\brief  return a tag identifying this VertexInputLayoutElement
		//	\date	2022/08/30 12:49
		const char* calcTag() const noexcept;
	};
public:
	template<VertexInputLayoutElementType layoutType>
	const VertexInputLayoutElement& fetch() const cond_noex
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
	const VertexInputLayoutElement& getElementByIndex( const size_t i ) const cond_noex;
	VertexInputLayout& add( const VertexInputLayoutElementType type ) cond_noex;
	const size_t getSizeInBytes() const cond_noex;
	const size_t getElementCount() const noexcept;
	std::vector<D3D11_INPUT_ELEMENT_DESC> getD3DInputElementDescs() const cond_noex;
	std::string calcSignature() const cond_noex;
	bool hasType( const VertexInputLayoutElementType& type ) const noexcept;
};//VertexInputLayout


class VertexView final
{
	friend class Buffer;

	char *m_p = nullptr;
	const VertexInputLayout &m_layout;
private:
	template<VertexInputLayout::VertexInputLayoutElementType layoutType>
	struct ElementExec final
	{
		//===================================================
		//	\function	exec
		//	\brief  returns the D3D11 DESC of INPUT_ELEMENT_LAYOUT
		//	\date	2022/08/30 12:49
		template<typename T>
		static constexpr auto exec( VertexView *pVertex,
			char *pElement,
			T &&val ) cond_noex
		{
			return pVertex->setElement<layoutType>( pElement,
				std::forward<T>( val ) );
		}
	};
public:
	//===================================================
	//	\function	element
	//	\brief  get reference to VertexInputLayoutElement
	//	\date	2022/08/30 12:45
	template<VertexInputLayout::VertexInputLayoutElementType layoutType>
	auto& element() cond_noex
	{
		auto pElement = m_p + m_layout.fetch<layoutType>().getByteOffsetInLayout();
		return *reinterpret_cast<typename VertexInputLayout::VertexInputLayoutElementProperties<layoutType>::CPUType*>( pElement );
	}
protected:
	VertexView( char *pData, const VertexInputLayout &layout ) cond_noex;
private:
	//===================================================
	//	\function	setElementByIndex
	//	\brief  terminal case for setElementByIndex
	//	\date	2022/08/30 12:45
	template<typename T>
	void setElementByIndex( const size_t i,
		T &&val ) cond_noex
	{
		const auto &element = m_layout.getElementByIndex( i );
		auto pElement = m_p + element.getByteOffsetInLayout();
		VertexInputLayout::bridge<ElementExec>( element.getType(),
			this,
			pElement,
			std::forward<T>( val ) );
	}
	//===================================================
	//	\function	setElementByIndex
	//	\brief  set Vertex data by its Byte index in the buffer
	//	\date	2022/08/30 12:45
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
	//===================================================
	//	\function	setElement
	//	\brief  helper to reduce tag duplication in setElementByIndex
	//	\date	2022/08/30 12:46
	template<VertexInputLayout::VertexInputLayoutElementType layoutType, typename SrcType>
	void setElement( char *pElement,
		SrcType &&val ) cond_noex
	{
		using Dest = typename VertexInputLayout::VertexInputLayoutElementProperties<layoutType>::CPUType;
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
	VertexView m_vertexView;
public:
	ConstVertexView( const VertexView &v ) cond_noex;
	template<VertexInputLayout::VertexInputLayoutElementType layoutType>
	const auto& getElement() const cond_noex
	{
		return const_cast<VertexView&>( m_vertexView ).element<layoutType>();
	}
};


class Buffer final
{
	std::vector<char> m_data;
	VertexInputLayout m_vertexLayout;
public:
	Buffer( VertexInputLayout layout, const size_t size = 0u ) cond_noex;
	Buffer( VertexInputLayout layout, const aiMesh &mesh );
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
	VertexView back() cond_noex;
	ConstVertexView back() const cond_noex;
	VertexView front() cond_noex;
	ConstVertexView front() const cond_noex;
	VertexView operator[]( const size_t i ) cond_noex;
	ConstVertexView operator[]( const size_t i ) const cond_noex;
};


}// namespace ver

#undef ASSIMP_EXTRACT_AIMESH
#ifndef VB_IMPL_SOURCE
#undef VERTEX_INPUT_LAYOUT_ELEMENT_TYPES
#endif