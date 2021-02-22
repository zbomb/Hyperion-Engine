/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/RDX11Resource.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class RDX11Resource
	{

	public:

		virtual ID3D11ShaderResourceView* GetSRV() = 0;
		virtual ID3D11UnorderedAccessView* GetUAV() = 0;
		virtual ID3D11RenderTargetView* GetRTV() = 0;
		virtual ID3D11ShaderResourceView** GetSRVAddress() = 0;
		virtual ID3D11UnorderedAccessView** GetUAVAddress() = 0;
		virtual ID3D11RenderTargetView** GetRTVAddress() = 0;
		virtual ID3D11Resource* GetResource() = 0;

	};

}