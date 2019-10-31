/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Point.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


namespace Hyperion
{
	template< typename T >
	class Point
	{

	public:

		T X;
		T Y;

		Point()
		{
			X = T();
			Y = T();
		}

		Point( T inX, T inY )
			: X( inX ), Y( inY )
		{
		}

	};

}