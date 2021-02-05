/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11.h
	© 2019, Zachary Berry
==================================================================================================*/

/*
	This contains all of the includes and links needed for DirectX11
*/

#pragma once

// Link in directx 11 libs
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
//#pragma comment( lib, "DirectXTK.lib" )

#include <Windows.h>

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11_2.h>
#include <DirectXMath.h>

#include <wrl.h>


namespace Hyperion
{

	constexpr uint32 DX11_LIGHT_TYPE_POINT			= 0;
	constexpr uint32 DX11_LIGHT_TYPE_SPOT			= 1;
	constexpr uint32 DX11_LIGHT_TYPE_DIRECTIONAL	= 2;

	/*
	*	Structures
	*/
	struct MatrixBuffer
	{
		DirectX::XMMATRIX World;
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
	};
	

	struct CamerBuffer
	{
		DirectX::XMFLOAT3 Position;
		float _RSVD_;
	};

	#pragma pack( push, 1 )
	struct LightInfo
	{
		DirectX::XMFLOAT3 WorldPosition;
		float AttnRadius;
		DirectX::XMFLOAT3 Color;
		float Brightness;
		uint32 Type;
		DirectX::XMFLOAT3 Direction;
		float SpotFOV;
		DirectX::XMFLOAT3 _pad;
	};
	#pragma pack( pop )

}