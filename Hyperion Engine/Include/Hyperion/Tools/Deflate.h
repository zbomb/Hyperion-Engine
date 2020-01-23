/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/Deflate.h
	© 2019, Zachary Berry
==================================================================================================*/

/*
	Deflate Wrapper Library
*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
namespace Tools
{

	class Deflate
	{

	public:

		Deflate() = delete;

		static bool PerformDeflate( const std::vector< byte >& Source, std::vector< byte >& Output, uint8 CompressionLevel = 6 );
		static bool PerformInflate( const std::vector< byte >& Source, std::vector< byte >& Output );

		static bool PerformDeflate( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< byte >& Output, uint8 CompressionLevel = 6 );
		static bool PerformInflate( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< byte >& Output );

	};

}
}