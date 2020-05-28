/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	� 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;


namespace Hyperion
{
	public enum class BC7Result
	{
		Success = 0,
		InvalidParams = 1,
		OtherError = 2
	};

	public ref class BC7Library
	{

	public:

		static BC7Result Encode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, bool bIncludeAlpha, [Out] cli::array< Byte >^% outData, [Out] UInt32% outRowSize );

	};

}