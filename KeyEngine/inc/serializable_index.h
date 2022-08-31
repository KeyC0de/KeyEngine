#pragma once

#include "non_copyable.h"
#include <unordered_map>


template<typename T>
class SerializableIndexTable;

template<typename T>
class SerializableIndexedObject;

static constexpr unsigned invalidSerializableIndex = 0;

template<typename T>
class SerializableIndex
	:
	NonCopyable
{
	friend class SerializableIndexedObject<T>;
	friend class SerializableIndexTable<T>;

	unsigned m_i;
public:
	explicit SerializableIndex( unsigned index )
		:
		m_i(index)
	{

	}

	SerializableIndex( SerializableIndex &&rhs )
		:
		m_i(rhs.m_i)
	{
		rhs.m_i = invalidSerializableIndex;
	}

	explicit const operator unsigned() const noexcept
	{
		return m_i;
	}
};

template<typename T>
class SerializableIndexTable
	:
	public NonCopyable
{
	std::unordered_map<unsigned, const saferp T*> m_table;
	unsigned m_nextIndex;
	friend class SerializableIndexedObject<T>;

	SerializableIndex<T> add( const T& obj )
	{
		SerializableIndex<T> index{m_nextIndex};
		m_table.insert( std::make_pair( index.m_i,
			&obj ) );
		++m_nextIndex;
		return index;
	}

	void update( unsigned index,
		const T& obj )
	{
		m_table.insert( std::make_pair( index,
			&obj ) );
	}

	void remove( const SerializableIndex<T> &id )
	{
		m_table.erase( id.m_i );
	}
public:
	SerializableIndexTable()
		:
		m_table(invalidSerializableIndex, 0xFFFFFFFF, 50),
		m_nextIndex(1)
	{

	}

	SerializableIndexTable( SerializableIndexTable &&table )
		:
		m_table(std::move( table.m_table )),
		m_nextIndex(table.m_nextIndex)
	{

	}

	const T* get( const SerializableIndex<T> &id ) const
	{
		if ( id.m_i == invalidSerializableIndex )
		{
			return nullptr;
		}
		const T *const *lookup = m_table.at( id.m_i );
		if ( lookup != nullptr )
		{
			return *lookup;
		}
		return nullptr;
	}

	const unsigned nextIndex( void ) const noexcept
	{
		return m_nextIndex;
	}
};

template<typename T>
class SerializableIndexedObject
	:
	public NonCopyable
{
	SerializableIndex<T> m_index;
	SerializableIndexTable<T> *m_table;
public:
	SerializableIndexedObject( SerializableIndexTable<T> &table,
		const T &obj )
		:
		m_index(table.add( obj )),
		m_table(table)
	{

	}

	SerializableIndexedObject( SerializableIndexedObject &&obj )
		:
		m_index(std::move( obj.m_index )),
		m_table(obj.m_table)
	{
		m_table->update( m_index.m_i,
			*reinterpret_cast<T*>( this ) );
	}

	virtual ~SerializableIndexedObject()
	{
		if ( m_index.m_i != invalidSerializableIndex )
		{
			m_table->remove( m_index );
		}
	}

	const SerializableIndex<T>& getIndex() const noexcept
	{
		return m_index;
	}
};

template<typename T>
class COMMAND_QUEUE_INDEXED_OBJECT
	:
	public SERIALISABLE_INDEXED_OBJECT<T>
{
public:
	COMMAND_QUEUE_INDEXED_OBJECT( COMMAND_QUEUE_INDEX_TABLE<T>& table, const T& this_obj, card32 force_index = 0 )
		:
		SERIALISABLE_INDEXED_OBJECT<T>( table, this_obj, force_index )
	{
	}
	COMMAND_QUEUE_INDEXED_OBJECT( EmpireFileInSection & section, COMMAND_QUEUE_INDEX_TABLE<T>& table, const T& this_obj )
		:
		SERIALISABLE_INDEXED_OBJECT<T>( section, table, this_obj )
	{
	}
	COMMAND_QUEUE_INDEXED_OBJECT( COMMAND_QUEUE_INDEXED_OBJECT && obj )
		:
		SERIALISABLE_INDEXED_OBJECT<T>( std::move( obj ) )
	{
	}
	const COMMAND_QUEUE_INDEX<T> & command_queue_index() const
	{
		return static_cast<const COMMAND_QUEUE_INDEX<T> &>( SERIALISABLE_INDEXED_OBJECT<T>::serialisable_index() );
	}

	// These offsets are to ensure that a COMMAND_QUEUE_INDEXED_OBJECT has an id that is 'GLOBALLY UNIQUE'. As currently unique_id() is just unique among objects of same type, but ui needs them to be globally unique for selection
	// I'm sure there is a better way of doing this...
	card32 cqi_offset_unit()
	{
		return 1000;
	}
	card32 cqi_offset_ship()
	{
		return 2000;
	}
	card32 cqi_offset_deployable()
	{
		return 3000;
	}
	card32 cqi_offset_building()
	{
		return 4000;
	}
};
