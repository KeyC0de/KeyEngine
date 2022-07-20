#include "snake.h"
#include "graphics.h"
#include "snake_play_field.h"
#include "assertions_console.h"


SnakeRepr::SnakeRepr( const GridLocation& gridLoc )
{
	m_segments.emplace_back( gridLoc );
}

void SnakeRepr::render( Graphics &gph,
	SnakePlayField& field ) const
{
	for ( const auto& s : m_segments )
	{
		s.render( gph,
			field );
	}
}

void SnakeRepr::moveRel( const GridLocation& delta )
{
	for ( int i = m_segments.size() - 1; i > 0; --i )
	{
		m_segments[i].follow( m_segments[i - 1] );
	}
	m_segments.front().advance( delta );
}

GridLocation SnakeRepr::getNextHeadLocation( const GridLocation& delta ) const
{
	GridLocation gridLoc{m_segments[0].getGridLocation()};
	gridLoc += delta;
	return gridLoc;
}

void SnakeRepr::grow( const GridLocation& delta )
{
	m_segments.emplace_back( *(new Segment{bodyColor}) );
	moveRel( delta );
}

bool SnakeRepr::checkForCollisions( const GridLocation& targetGridLoc ) const
{
	for ( int i = 0; i < m_segments.size() - 1; ++i )
	{
		if ( m_segments[i].getGridLocation() == targetGridLoc )
		{
			return true;
		}
	}
	return false;
}

int SnakeRepr::getLength() const
{
	return m_segments.size();
}

SnakeRepr::Segment::Segment( const GridLocation& gridLoc )
{
	m_gridLoc = gridLoc;
	m_color = SnakeRepr::headColor;
}

SnakeRepr::Segment::Segment( ColorBGRA col )
{
	this->m_color = col;
}

void SnakeRepr::Segment::render( Graphics &gph,
	SnakePlayField& field ) const
{
	field.renderCell( gph,
		m_gridLoc,
		m_color );
}

void SnakeRepr::Segment::advance( const GridLocation& delta )
{
	ASSERT( abs( delta.x ) + abs( delta.y ) == 1, "A segment can move 1 pixel at a time!" );
	m_gridLoc += delta;
}

void SnakeRepr::Segment::follow( const Segment& next )
{
	m_gridLoc = next.m_gridLoc;
}

const GridLocation& SnakeRepr::Segment::getGridLocation() const
{
	return this->m_gridLoc;
}