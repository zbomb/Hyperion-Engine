/*==================================================================================================
	Hyperion Engine
	Source/Library/Math.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Math.h"


namespace Hyperion
{
	namespace Math
	{

		float PIf( 3.1415926535897932384626433f );
		double PId( 3.1415926535897932384626433 );


		uint32 Pow( uint32 base, uint32 exp )
		{
			uint32 output = 1;

			for( uint32 i = 0; i < exp; i++ )
			{
				if( i == 0 )
					output = base;
				else
					output *= base;
			}

			return output;
		}

	}
}