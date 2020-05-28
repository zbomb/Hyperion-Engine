/*==================================================================================================
	Hyperion Engine
	Source/Library/Crypto.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Crypto.h"


namespace Hyperion
{

	uint32 Crypto::RSHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd )
	{
		const uint32 b	= 378551;
		uint32 a		= 63689;
		uint32 output	= 0;

		for( auto It = inBegin; It != inEnd; It++ )
		{
			output	= output * a + *It;
			a		*= b;
		}

		return output;
	}


	uint32 Crypto::JSHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd )
	{
		uint32 hash = 1315423911;

		for( auto It = inBegin; It != inEnd; It++ )
		{
			hash ^= ( ( hash << 5 ) + ( *It ) + ( hash >> 2 ) );
		}

		return hash;
	}


	uint32 Crypto::PJWHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd )
	{
		const uint32 highBits		= 0xFFFFFFFF << 28;

		uint32 output = 0;
		uint32 test = 0;
		uint32 i = 0;

		for( auto It = inBegin; It != inEnd; It++ )
		{
			output = ( output << 4 ) + ( *It );

			if( ( test = output & highBits ) != 0 )
			{
				output = ( ( output ^ ( test >> 24 ) )& ( ~highBits ) );
			}
		}

		return output;
	}


	uint32 Crypto::ELFHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd )
	{
		uint32 output = 0;
		uint32 x = 0;

		for( auto It = inBegin; It != inEnd; It++ )
		{
			output = ( output << 4 ) + ( *It );
			if( ( x = output & 0xF0000000 ) != 0 )
			{
				output ^= ( x >> 24 );
			}

			output &= ~x;
		}

		return output;
	}


}