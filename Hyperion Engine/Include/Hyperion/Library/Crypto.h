/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Library/Crypto.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	class Crypto
	{

	public:

		static uint32 RSHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd );
		inline static uint32 RSHash( const std::vector< byte >& inData ) { return RSHash( inData.begin(), inData.end() ); }

		static uint32 JSHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd );
		inline static uint32 JSHash( const std::vector< byte >& inData ) { return JSHash( inData.begin(), inData.end() ); }

		static uint32 PJWHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd );
		inline static uint32 PJWHash( const std::vector< byte >& inData ) { return PJWHash( inData.begin(), inData.end() ); }

		static uint32 ELFHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd );
		inline static uint32 ELFHash( const std::vector< byte >& inData ) { return ELFHash( inData.begin(), inData.end() ); }


	};

}