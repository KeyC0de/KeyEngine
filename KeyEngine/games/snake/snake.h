#pragma once

#include <vector>
#include "grid_location.h"
#include "color.h"


class Graphics;
class SnakePlayField;

class SnakeRepr
{
	static inline ColorBGRA headColor = col::White;
	static inline ColorBGRA bodyColor = col::DarkGreen;

	class Segment
	{
		GridLocation m_gridLoc;
		ColorBGRA m_color;
	public:
		Segment( const GridLocation &gridLoc );
		Segment( ColorBGRA col );

		void render( Graphics &gph, SnakePlayField &field ) const;
		void follow( const Segment &next );
		void advance( const GridLocation &delta );
		const GridLocation &getGridLocation() const;
	};

	std::vector<Segment> m_segments;
public:
	SnakeRepr( const GridLocation &gridLoc );

	void render( Graphics &gph, SnakePlayField &field ) const;
	void moveRel( const GridLocation &delta );
	void grow( const GridLocation &delta );
	GridLocation getNextHeadLocation( const GridLocation &delta ) const;
	//===================================================
	//	\function	checkForCollisions
	//	\brief  if a segment's location is equal to the next head location then collision!
	//	\date	2021/10/24 20:09
	bool checkForCollisions( const GridLocation &targetGridLoc ) const;
	int getLength() const;
};