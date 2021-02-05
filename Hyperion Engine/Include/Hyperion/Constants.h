/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Constants.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Ints.h"
#include <string>


namespace Hyperion
{

	class GameInstance;

	namespace Encoding
	{
		constexpr uint32 InvalidChar = 0xFFFD;
	}

	enum class GraphicsAPI
	{
		None = 0,
		DX11 = 1,
		DX12 = 2,
		OpenGL = 3
	};

	enum class LightType
	{
		Point = 1,
		Spot = 2,
		Directional = 3
	};

	typedef unsigned int ObjectID;
	constexpr ObjectID OBJECT_INVALID	= 0;
	constexpr ObjectID OBJECT_NONE		= 0;

	// TODO: Check if this is deprecated
	typedef unsigned int ObjectCacheID;
	constexpr ObjectCacheID CACHE_INVALID		= 0;
	constexpr ObjectCacheID CACHE_NONE			= 0;
	constexpr ObjectCacheID CACHE_TEST			= 1;
	constexpr ObjectCacheID CACHE_OTHER			= 2;
	constexpr ObjectCacheID CACHE_CORE			= 3;
	constexpr ObjectCacheID CACHE_ENTITY		= 4;
	constexpr ObjectCacheID CACHE_COMPONENT		= 5;
	constexpr ObjectCacheID CACHE_RENDERER		= 6;

	typedef uint16 LanguageID;
	constexpr LanguageID LANG_NONE = 0;

	constexpr uint32 ASSET_INVALID			= 0;
	constexpr uint32 ASSET_TYPE_INVALID		= 0;

	constexpr uint32 PLAYER_LOCAL = 0;
	constexpr uint32 PLAYER_INVALID = std::numeric_limits< uint32 >::max();

	constexpr uint32 ASSET_TYPE_TEXTURE			= 1;
	constexpr uint32 ASSET_TYPE_STATICMODEL		= 2;
	constexpr uint32 ASSET_TYPE_MATERIAL		= 3;

	constexpr auto THREAD_GAME		= "game";
	constexpr auto THREAD_RENDERER	= "renderer";
	constexpr auto THREAD_POOL		= "pool";

	constexpr uint32 RENDERER_CLUSTER_COUNT_X		= 15;
	constexpr uint32 RENDERER_CLUSTER_COUNT_Y		= 10;
	constexpr uint32 RENDERER_CLUSTER_COUNT_Z		= 24;
	constexpr uint32 RENDERER_CLUSTER_MAX_LIGHTS	= 128;
	constexpr uint32 RENDERER_MAX_DYNAMIC_LIGHTS	= 262144;	// This limit is due to the compute function that assigns lights to view clusters
																// The lights are processed by 512 threads per cluster, with a maximum of 512 lights per thread, giving us this value


	// Flags 
	constexpr uint32 FLAG_NONE				= 0U;

	constexpr uint32 FLAG_RENDERER_DX11		= 0b00000000'00000000'00000001'00000000;
	constexpr uint32 FLAG_RENDERER_DX12		= 0b00000000'00000000'00000010'00000000;
	constexpr uint32 FLAG_RENDERER_OGL		= 0b00000000'00000000'00000100'00000000;
	constexpr uint32 FLAG_RENDERER_VSYNC	= 0b00000000'00000000'00001000'00000000;

	// Default Resolution
	// TODO: Make this dynamic, select a default using the graphics api to see whats available, monitor aspect ratio, etc...
	constexpr uint32 DEFAULT_RESOLUTION_WIDTH		= 1080;
	constexpr uint32 DEFAULT_RESOLUTION_HEIGHT		= 720;
	constexpr bool DEFAULT_RESOLUTION_FULLSCREEN	= false;

	constexpr uint32 MIN_RESOLUTION_WIDTH	= 480;
	constexpr uint32 MIN_RESOLUTION_HEIGHT	= 360;

	/*
	*	Static Settings
	*	- Eventually we need to create a StaticVar system, where we can set vars from some time of external program
	*	- Basically, we have a header file (not included in the project file) and is included by the main header
	*	- Then, settings are saved into this file for build
	*/
	constexpr auto TYPE_OVERRIDE_GAME_INSTANCE	= "gameinstance";
	
	constexpr uint32 DEFAULT_API_WIN32			= FLAG_RENDERER_DX11;
	constexpr uint32 DEFAULT_API_OSX			= FLAG_RENDERER_OGL;

	constexpr auto SHADER_PATH_DX11_VERTEX_SCENE			= "shaders/dx11/scene.hvs";
	constexpr auto SHADER_PATH_DX11_VERTEX_SCREEN			= "shaders/dx11/screen.hvs";
	constexpr auto SHADER_PATH_DX11_PIXEL_GBUFFER			= "shaders/dx11/gbuffer.hps";
	constexpr auto SHADER_PATH_DX11_PIXEL_LIGHTING			= "shaders/dx11/lighting.hps";
	constexpr auto SHADER_PATH_DX11_COMPUTE_BUILD_CLUSTERS	= "shaders/dx11/build_clusters.hcs";
	constexpr auto SHADER_PATH_DX11_COMPUTE_FIND_CLUSTERS	= "shaders/dx11/find_clusters.hcs";
	constexpr auto SHADER_PATH_DX11_COMPUTE_CULL_LIGHTS		= "shaders/dx11/cull_lights.hcs";
	constexpr auto SHADER_PATH_DX11_PIXEL_FORWARD_PRE_Z		= "shaders/dx11/forward_pre_z.hps";
	constexpr auto SHADER_PATH_DX11_PIXEL_FORWARD			= "shaders/dx11/forward.hps";

	constexpr uint32 BUILD_CLUSTERS_MODE_REBUILD	= 0;
	constexpr uint32 BUILD_CLUSTERS_MODE_CLEAR		= 1;

	// Enums
	enum class GeometryCollectionSource
	{
		Scene = 0,
		ScreenQuad = 1
	};

	enum class PipelineRenderTarget
	{
		Screen = 0,
		GBuffer = 1,
		ViewClusters = 2
	};

	enum class PipelineDepthStencilTarget
	{
		Screen		= 0,
		GBuffer		= 1,
		None		= 3
	};

	// Flags
	constexpr uint32 RENDERER_GEOMETRY_COLLECTION_FLAG_NONE			= 0;
	constexpr uint32 RENDERER_GEOMETRY_COLLECTION_FLAG_OPAQUE		= 1;
	constexpr uint32 RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT	= 2;

	// Shader Types
	enum class VertexShaderType
	{
		Scene	= 0,
		Screen	= 1
	};

	enum class PixelShaderType
	{
		GBuffer			= 0,
		Lighting		= 1,
		ForwardPreZ		= 2,
		Forward			= 3
	};

	enum class GeometryShaderType
	{

	};

	enum class ComputeShaderType
	{
		BuildClusters	= 0,
		FindClusters	= 1,
		CullLights		= 2
	};

	/*
		Maximum number of LODs that a texture can have, this makes the max texture width/height is 65,536px
	*/
	constexpr uint8 TEXTURE_MAX_LODS = 15;
	constexpr uint8 MODEL_MAX_LODS = 10;

	enum class TextureFormat
	{
		NONE = 0,

		/*
		 * 8-bit Types
		*/
		// Unsigned Normals (converted to floats in shaders) [0,1]
		R_8BIT_UNORM = 1,
		RG_8BIT_UNORM = 2,
		RGBA_8BIT_UNORM = 4,
		RGBA_8BIT_UNORM_SRGB = 5,

		// Signed Normals (converted to floats in shaders) [-1,1]
		R_8BIT_SNORM = 6,
		RG_8BIT_SNORM = 7,
		RGBA_8BIT_SNORM = 9,

		// Unsigned Integers (not converted to floats)
		R_8BIT_UINT = 10,
		RG_8BIT_UINT = 11,
		RGBA_8BIT_UINT = 13,

		// Signed Integers (not converted to floats)
		R_8BIT_SINT = 14,
		RG_8BIT_SINT = 15,
		RGBA_8BIT_SINT = 17,

		/*
		 * 16-bit Types
		*/
		// Unsigned normals (converted to floats in shaders) [0,1]
		R_16BIT_UNORM = 18,
		RG_16BIT_UNORM = 19,
		RGBA_16BIT_UNORM = 21,

		// Signed normals (converted to floats in shaders) [-1,1]
		R_16BIT_SNORM = 23,
		RG_16BIT_SNORM = 24,
		RGBA_16BIT_SNORM = 26,

		// Unisnged Integers (not converted to floats)
		R_16BIT_UINT = 27,
		RG_16BIT_UINT = 28,
		RGBA_16BIT_UINT = 30,

		// Signed Integers (not converted to floats)
		R_16BIT_SINT = 31,
		RG_16BIT_SINT = 32,
		RGBA_16BIT_SINT = 34,

		// Floats 
		R_16BIT_FLOAT = 35,
		RG_16BIT_FLOAT = 36,
		RGBA_16BIT_FLOAT = 38,

		/*
		 * 32-bit Types
		*/

		// Unsigned integers (not converted to floats)
		R_32BIT_UINT = 48,
		RG_32BIT_UINT = 49,
		RGB_32BIT_UINT = 50,
		RGBA_32BIT_UINT = 51,

		// Signed integers (not converted to floats)
		R_32BIT_SINT = 52,
		RG_32BIT_SINT = 53,
		RGB_32BIT_SINT = 54,
		RGBA_32BIT_SINT = 55,

		// Floats
		R_32BIT_FLOAT = 56,
		RG_32BIT_FLOAT = 57,
		RGB_32BIT_FLOAT = 58,
		RGBA_32BIT_FLOAT = 59,

		/*
			Compressed Types
		*/
		RGB_DXT_1 = 60,
		RGBA_DXT_5 = 61,
		RGB_BC_7 = 62,
		RGBA_BC_7 = 63

		/*	
			Valid Texture Asset Formats
			
			1. RGB_DXT_1
			2. RGBA_DXT_5
			3. RGB_BC_7
			4. RGBA_BC_7
			5. R_8BIT_UNORM
			6. RG_8BIT_UNORM
			7. RGBA_8BIT_UNORM
			8. RGBA_8BIT_UNORM_SRGB
		*/
	};

	static bool IsValidTextureAssetFormat( TextureFormat inFormat )
	{
		switch( inFormat )
		{
		case TextureFormat::RGB_DXT_1:
		case TextureFormat::RGBA_DXT_5:
		case TextureFormat::RGB_BC_7:
		case TextureFormat::RGBA_BC_7:
		case TextureFormat::R_8BIT_UNORM:
		case TextureFormat::RG_8BIT_UNORM:
		case TextureFormat::RGBA_8BIT_UNORM:
		case TextureFormat::RGBA_8BIT_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	enum class InputAxis
	{
		None	= 0,
		MouseX	= 1,
		MouseY	= 2
	};

	enum class KeyEvent
	{
		None		= 0,
		Pressed		= 1,
		Released	= 2,
		Any			= 3
	};

	enum class Keys
	{
		NONE = 0,

		// Letter Keys
		A	= 1,
		B	= 2,
		C	= 3,
		D	= 4,
		E	= 5,
		F	= 6,
		G	= 7,
		H	= 8,
		I	= 9,
		J	= 10,
		K	= 11,
		L	= 12,
		M	= 13,
		N	= 14,
		O	= 15,
		P	= 16,
		Q	= 17,
		R	= 18,
		S	= 19,
		T	= 20,
		U	= 21,
		V	= 22,
		W	= 23,
		X	= 24,
		Y	= 25,
		Z	= 26,

		// Modifier Keys
		TAB			= 123,
		CAPS		= 27,
		LSHIFT		= 29,
		RSHIFT		= 30,
		ENTER		= 31,
		LCTRL		= 32,
		LALT		= 33,
		SPACE		= 34,
		BACKSPACE	= 35,

		// Arrows
		UP		= 36,
		LEFT	= 37,
		RIGHT	= 38,
		DOWN	= 39,

		// Tilde
		TILDE	= 40,

		// Number Keys
		ONE		= 41,
		TWO		= 42,
		THREE	= 43,
		FOUR	= 44,
		FIVE	= 45,
		SIX		= 46,
		SEVEN	= 47,
		EIGHT	= 48,
		NINE	= 49,
		ZERO	= 50,

		// Character Keys
		MINUS			= 51,
		EQUALS			= 52,
		LBRACKET		= 53,
		RBRACKET		= 54,
		FORWARDSLASH	= 55,
		SEMICOLON		= 56,
		QUOTE			= 57,
		COMMA			= 58,
		PERIOD			= 59,
		BACKSLASH		= 60,

		// Function Keys
		F1		= 61,
		F2		= 62,
		F3		= 63,
		F4		= 64,
		F5		= 65,
		F6		= 66,
		F7		= 67,
		F8		= 68,
		F9		= 69,
		F10		= 70,
		F11		= 71,
		F12		= 72,

		// Other Keys
		INSERT			= 73,
		HOME			= 74,
		PAGEUP			= 75,
		PAGEDOWN		= 76,
		DEL				= 77,
		END				= 78,
		PRINTSCREEN		= 79,
		SCROLL_LOCK		= 80,
		BREAK			= 81,

		// Numpad Stuff
		NUM0		= 82,
		NUM1		= 83,
		NUM2		= 84,
		NUM3		= 85,
		NUM4		= 86,
		NUM5		= 87,
		NUM6		= 88,
		NUM7		= 89,
		NUM8		= 90,
		NUM9		= 91,
		NUM_DIV		= 92,
		NUM_MULT	= 93,
		NUM_SUB		= 94,
		NUM_ADD		= 95,
		NUM_ENTER	= 96,
		NUM_DECIMAL	= 97,

		// Escape Key
		ESCAPE	= 98,

		// Mouse Stuff
		MOUSE1		= 99,
		MOUSE2		= 100,
		MOUSE3		= 101,
		MOUSE4		= 102,
		MOUSE5		= 103,
		SCROLLUP	= 104,
		SCROLLDOWN	= 105,

		// Misc Keys
		CLEAR	= 106,
		PAUSE	= 107,

		// Extended Function Keys
		F13		= 108,
		F14		= 109,
		F15		= 110,
		F16		= 111,
		F17		= 112,
		F18		= 113,
		F19		= 114,
		F20		= 115,
		F21		= 116,
		F22		= 117,
		F23		= 118,
		F24		= 119,

		// Other Keys I forgot about..
		NUM_LOCK		= 120,
		RCTRL			= 121,
		RALT			= 122

	};

	enum class ByteOrder
	{
		LittleEndian = 0,
		BigEndian = 1
	};

	/*
		static std::string GetKeyName( Hyperion::Keys )
	*/
	static std::string GetKeyName( Hyperion::Keys Input )
	{
		switch( Input )
		{
		case Keys::A:
			return "A";
		case Keys::B:
			return "B";
		case Keys::BACKSLASH:
			return "\\";
		case Keys::BACKSPACE:
			return "Backspace";
		case Keys::BREAK:
			return "Break";
		case Keys::C:
			return "C";
		case Keys::CAPS:
			return "Caps";
		case Keys::CLEAR:
			return "Clear";
		case Keys::COMMA:
			return ",";
		case Keys::D:
			return "D";
		case Keys::DEL:
			return "Del";
		case Keys::DOWN:
			return "Down";
		case Keys::END:
			return "End";
		case Keys::E:
			return "E";
		case Keys::EIGHT:
			return "8";
		case Keys::ENTER:
			return "Enter";
		case Keys::EQUALS:
			return "=";
		case Keys::ESCAPE:
			return "Esc";
		case Keys::F:
			return "F";
		case Keys::F1:
			return "F1";
		case Keys::F2:
			return "F2";
		case Keys::F3:
			return "F3";
		case Keys::F4:
			return "F4";
		case Keys::F5:
			return "F5";
		case Keys::F6:
			return "F6";
		case Keys::F7:
			return "F7";
		case Keys::F8:
			return "F8";
		case Keys::F9:
			return "F9";
		case Keys::F10:
			return "F10";
		case Keys::F11:
			return "F11";
		case Keys::F12:
			return "F12";
		case Keys::F13:
			return "F13";
		case Keys::F14:
			return "F14";
		case Keys::F15:
			return "F15";
		case Keys::F16:
			return "F16";
		case Keys::F17:
			return "F17";
		case Keys::F18:
			return "F18";
		case Keys::F19:
			return "F19";
		case Keys::F20:
			return "F20";
		case Keys::F21:
			return "F21";
		case Keys::F22:
			return "F22";
		case Keys::F23:
			return "F23";
		case Keys::F24:
			return "F24";
		case Keys::FIVE:
			return "Five";
		case Keys::FORWARDSLASH:
			return "/";
		case Keys::FOUR:
			return "4";
		case Keys::G:
			return "G";
		case Keys::H:
			return "H";
		case Keys::HOME:
			return "Home";
		case Keys::I:
			return "I";
		case Keys::INSERT:
			return "Insert";
		case Keys::J:
			return "J";
		case Keys::K:
			return "K";
		case Keys::L:
			return "L";
		case Keys::LALT:
			return "L ALt";
		case Keys::LBRACKET:
			return "[";
		case Keys::LCTRL:
			return "L Ctrl";
		case Keys::LEFT:
			return "Left";
		case Keys::LSHIFT:
			return "L Shift";
		case Keys::M:
			return "M";
		case Keys::MINUS:
			return "Minus";
		case Keys::MOUSE1:
			return "Mouse 1";
		case Keys::MOUSE2:
			return "Mouse 2";
		case Keys::MOUSE3:
			return "Mouse 3";
		case Keys::MOUSE4:
			return "Mouse 4";
		case Keys::MOUSE5:
			return "Mouse 5";
		case Keys::N:
			return "N";
		case Keys::NINE:
			return "9";
		case Keys::NONE:
			return "NONE";
		case Keys::NUM0:
			return "Num 0";
		case Keys::NUM1:
			return "Num 1";
		case Keys::NUM2:
			return "Num 2";
		case Keys::NUM3:
			return "Num 3";
		case Keys::NUM4:
			return "Num 4";
		case Keys::NUM5:
			return "Num 5";
		case Keys::NUM6:
			return "Num 6";
		case Keys::NUM7:
			return "Num 7";
		case Keys::NUM8:
			return "Num 8";
		case Keys::NUM9:
			return "Num 9";
		case Keys::NUM_ADD:
			return "Num Add";
		case Keys::NUM_DECIMAL:
			return "Num Decimal";
		case Keys::NUM_DIV:
			return "Num Div";
		case Keys::NUM_ENTER:
			return "Num Enter";
		case Keys::NUM_LOCK:
			return "Num Lock";
		case Keys::NUM_MULT:
			return "Num Mult";
		case Keys::NUM_SUB:
			return "Num Subtract";
		case Keys::O:
			return "O";
		case Keys::ONE:
			return "1";
		case Keys::P:
			return "P";
		case Keys::PAGEDOWN:
			return "Page Down";
		case Keys::PAGEUP:
			return "Page Up";
		case Keys::PAUSE:
			return "Pause";
		case Keys::PERIOD:
			return ".";
		case Keys::PRINTSCREEN:
			return "Print Screen";
		case Keys::Q:
			return "Q";
		case Keys::QUOTE:
			return "\"";
		case Keys::R:
			return "R";
		case Keys::RALT:
			return "R Alt";
		case Keys::RBRACKET:
			return "]";
		case Keys::RCTRL:
			return "R Ctrl";
		case Keys::RIGHT:
			return "Right";
		case Keys::RSHIFT:
			return "R Shift";
		case Keys::S:
			return "S";
		case Keys::SCROLLDOWN:
			return "Scroll Down";
		case Keys::SCROLLUP:
			return "Scroll Up";
		case Keys::SCROLL_LOCK:
			return "Scroll Lock";
		case Keys::SEMICOLON:
			return ";";
		case Keys::SEVEN:
			return "7";
		case Keys::SIX:
			return "6";
		case Keys::SPACE:
			return "Space";
		case Keys::T:
			return "T";
		case Keys::TAB:
			return "Tab";
		case Keys::THREE:
			return "3";
		case Keys::TILDE:
			return "`";
		case Keys::TWO:
			return "2";
		case Keys::U:
			return "U";
		case Keys::UP:
			return "Up";
		case Keys::V:
			return "V";
		case Keys::W:
			return "W";
		case Keys::X:
			return "X";
		case Keys::Y:
			return "Y";
		case Keys::Z:
			return "Z";
		case Keys::ZERO:
			return "0";
		default:
			return "Unknown";
		}
	}

	
}