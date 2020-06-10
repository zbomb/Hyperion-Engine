/*==================================================================================================
	Hyperion Engine
	Source/Library/Math.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Math/MathCore.h"

namespace Hyperion
{

	float Math::PIf( 3.1415926535897932384626433f );
	double Math::PId( 3.1415926535897932384626433 );


	uint32 Math::Pow( uint32 base, uint32 exp )
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