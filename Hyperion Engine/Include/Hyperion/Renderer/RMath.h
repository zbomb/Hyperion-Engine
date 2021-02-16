/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/RMath.h
	© 2021, Zachary Berry
==================================================================================================*/

/*
*	Render Math Library
* 
*	We are using a seperate math library for the renderer, its going to be interoperable with the normal math library
*	Its actually going to be a wrapper around DX11 math if available, otherwise we will use OpenGL
*	There are a few reaosons for doing things this way...
*	1. SSE 
*	2. Easy to feed DX11 with matricies
*	3. Already well written, and tested math library
*/

#pragma once

#include "Hyperion/Hyperion.h"

#ifdef HYPERION_SUPPORT_DIRECTX
#include "Hyperion/Renderer/DirectX11/DirectX11Math.h"
#else
// TODO: OpenGL math?
#endif