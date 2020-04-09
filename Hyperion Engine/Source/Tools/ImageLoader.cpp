/*==================================================================================================
	Hyperion Engine
	Source/Tools/ImageLoader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/ImageLoader.h"
#include "Hyperion/Tools/PNGReader.h"
#include "Hyperion/File/FilePath.h"


namespace Hyperion
{


	ImageFormat ImageLoader::DetectFormat( const String& inFileName )
	{
		// Validate string
		if( inFileName.IsWhitespaceOrEmpty() )
		{
			return ImageFormat::Unknown;
		}

		// Skim file extension
		FilePath path( inFileName );
		if( !path.HasExtension() )
		{
			return ImageFormat::Unknown;
		}

		// Get extension as lowercase string
		auto ext = path.Extension();
		auto ext_s = ext.ToLower();

		if( ext_s.IsWhitespaceOrEmpty() )
		{
			return ImageFormat::Unknown;
		}

		// Determine which format this file is
		if( ext_s.Equals( ".png" ) )
		{
			return ImageFormat::PNG;
		}
		else if( ext_s.Equals( ".dds" ) )
		{
			return ImageFormat::DDS;
		}
		else
		{
			return ImageFormat::Unknown;
		}
	}


	std::shared_ptr< RawImageData > ImageLoader::LoadGeneric( const String& inFileName, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		auto format = DetectFormat( inFileName );

		switch( format )
		{
		case ImageFormat::PNG:
			return LoadPNG( Begin, End );
		case ImageFormat::DDS:
			return LoadDDS( Begin, End );
		case ImageFormat::Unknown:
		default:

			Console::WriteLine( "[ERROR] ImageLoader: Attempt to load unknown image format.. '", inFileName, "'!" );
			return nullptr;
		}
	}


	std::shared_ptr< RawImageData > ImageLoader::LoadPNG( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		std::shared_ptr< RawImageData > Output;
		if( Tools::PNGReader::LoadFromMemory( Begin, End, Output, 1.0f ) == Tools::PNGReader::Result::Success )
		{
			return Output;
		}

		return nullptr;
	}


	std::shared_ptr< RawImageData > ImageLoader::LoadDDS( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return nullptr;
	}

}
