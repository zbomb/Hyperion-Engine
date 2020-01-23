/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Console/Console.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <functional>
#include <deque>
#include <shared_mutex>
#include <atomic>

#include "Hyperion/Core/String.h"
#include "Hyperion/Console/ConsoleVarInstance.h"


namespace Hyperion
{
	constexpr uint32 FLAG_CONSOLE_NONE			= 0;
	constexpr uint32 FLAG_CONSOLE_OS_OUTPUT		= 1;


	class Console
	{

	private:

		static std::atomic< bool > m_bRunning;
		static std::atomic< bool > m_bOSOutput;

		static std::shared_mutex m_VarListMutex;
		static std::map< String, std::shared_ptr< ConsoleVarInstanceBase > > m_Vars;

		static std::mutex m_PrintMutex;

		static bool CreateVar( const std::shared_ptr< ConsoleVarInstanceBase >& inVar );

		static void PerformWrite( const String& in )
		{
			if( m_bOSOutput )
			{
				std::cout << in;
			}

			// Do internal writing
		}

		static void PerformWrite( const char* in )
		{
			PerformWrite( ToString( in ) );
		}

		template< typename T >
		static void PerformWrite( const T& in )
		{
			PerformWrite( ToString( in ) );
		}

	public:

		Console() = delete;

		static bool Start( uint32 inFlags = FLAG_CONSOLE_NONE );
		static bool Stop();

		/*
			Updated Write Functions
		*/
		template< typename... Args >
		static void Write( Args&&... args )
		{
			// Check if console is running
			if( !m_bRunning )
				return;

			// Aquire lock
			std::lock_guard< std::mutex > lock( m_PrintMutex );

			// Loop through arguments, print each, some cool C++17 magic
			( ( PerformWrite( ToString( args ) ) ), ... );

		}

		template< typename... Args >
		static void WriteLine( Args&&... args )
		{
			Write( args..., "\n" );
		}

		static bool SetVar( const String& inKey, const String& strValue, bool bCallback = true );

		template< typename T >
		static bool SetVar( const String& inKey, const T& inValue, bool bCallback = true )
		{
			if( inKey.IsWhitespaceOrEmpty() )
			{
				WriteLine( "[ERROR] Console: Failed to set cvar.. the provided key was null or whitespace" );
				return false;
			}

			// Find this cvar
			std::shared_ptr< ConsoleVarInstanceBase > varInst;

			{
				std::shared_lock< std::shared_mutex > lock( m_VarListMutex );

				auto entry = m_Vars.find( inKey );
				if( entry == m_Vars.end() || !entry->second )
				{
					WriteLine( "[ERROR] Console: Failed to set '", inKey, " because a cvar with that key doesnt exist" );
					return false;
				}

				varInst = entry->second;
			}

			// Attempt to cast to the desired type
			std::shared_ptr< ConsoleVarInstance< T > > casted = std::dynamic_pointer_cast<ConsoleVarInstance< T >>( varInst );
			if( !casted )
			{
				WriteLine( "[ERROR] Console: Failed to set '", inKey, "' because the value type didnt match the variable type" );
				return false;
			}

			if( !casted->SetValue( inValue, bCallback ) )
			{
				WriteLine( "[ERROR] Console: Failed to set '", inKey, "' because the value was invalid" );
				return false;
			}

			WriteLine( "Console: CVar '", inKey, "' was set to '", inValue, "'" );
			return true;
		}

		static bool GetVarAsString( const String& inKey, String& outValue );

		template< typename T >
		static bool GetVar( const String& inKey, T& outVal )
		{
			if( inKey.IsWhitespaceOrEmpty() )
			{
				WriteLine( "[ERROR] Console: Failed to get cvar value, because the key was null" );
				return false;
			}

			std::shared_ptr< ConsoleVarInstanceBase > varInst;
			{
				std::shared_lock< std::shared_mutex > lock( m_VarListMutex );
				auto entry = m_Vars.find( inKey );
				if( entry == m_Vars.end() || !entry->second )
				{
					WriteLine( "[ERROR] Console: Failed to get value for '", inKey, "'. Cvar wasnt found" );
					return false;
				}

				varInst = entry->second;
			}

			// Attempt to cast to desired type
			auto casted = std::dynamic_pointer_cast<ConsoleVarInstance< T >>( varInst );
			if( !casted )
			{
				WriteLine( "[ERROR] Console: Failed to get value for '", inKey, "' because the value type was invalid" );
				return false;
			}

			return casted->GetValue();
		}
		

		template< typename > friend class ConsoleVar;

	};
}