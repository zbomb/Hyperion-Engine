/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Util/Constants.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once
#include <string>

namespace Hyperion
{
	namespace Encoding
	{
		constexpr uint32 InvalidChar = 0xFFFD;
	}

	typedef unsigned int ObjectID;
	constexpr ObjectID OBJECT_INVALID	= 0;
	constexpr ObjectID OBJECT_NONE		= 0;

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

	constexpr auto THREAD_GAME		= "game";
	constexpr auto THREAD_RENDERER	= "renderer";
	constexpr auto THREAD_POOL		= "pool";

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
		Released	= 2
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
		SHIFT		= 28,
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
		case Keys::SHIFT:
			return "Shift";
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