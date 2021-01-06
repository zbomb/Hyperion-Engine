/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/ImageLoader/ImageLoader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	
	enum class ImageFormat
	{
		Unknown = 0,
		DDS = 1,
		PNG = 2
	};


	class ImageLoader
	{

	public:

		ImageLoader() = delete;

		static ImageFormat DetectFormat( const String& inFileName );

		static std::shared_ptr< RawImageData > LoadPNG( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );
		static std::shared_ptr< RawImageData > LoadDDS( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );
		static std::shared_ptr< RawImageData > LoadGeneric( const String& inFilename, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

	};
	
}