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