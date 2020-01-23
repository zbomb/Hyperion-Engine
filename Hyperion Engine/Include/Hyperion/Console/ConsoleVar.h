/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Console/ConsoleVar.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Console/Console.h"
#include "Hyperion/Console/ConsoleVarInstance.h"


namespace Hyperion
{

	template< typename T >
	class ConsoleVar
	{

	public:

		ConsoleVar() = delete;

	};

	template<>
	class ConsoleVar< uint32 >
	{

	private:

		std::shared_ptr< ConsoleVarInstance< uint32 > > m_Instance;
		uint32 m_Default;

	public:

		ConsoleVar() = delete;
		ConsoleVar( const String& inKey, const String& inDescription, uint32 inDefault, uint32 inMin, uint32 inMax, 
					std::function< void( uint32 ) > Callback = nullptr, const std::string& callbackThread = THREAD_POOL )
			: m_Default( inDefault )
		{
			// Create new command and try to add it to the console
			auto newVar = std::make_shared< ConsoleVarInstance< uint32 > >( inKey, inDescription, inDefault, inMin, inMax, Callback, callbackThread );
			
			if( Console::CreateVar( newVar ) )
			{
				m_Instance = newVar;
			}
		}

		~ConsoleVar()
		{
			m_Instance.reset();
		}

		bool IsValid()
		{
			return m_Instance ? true : false;
		}

		uint32 GetValue()
		{
			return m_Instance ? m_Instance->GetValue() : m_Default;
		}

		bool SetValue( uint32 inValue )
		{
			return m_Instance ? m_Instance->SetValue( inValue, true ) : false;
		}

		uint32 GetDefault() const
		{
			return m_Default;
		}

		String GetKey()
		{
			return m_Instance ? m_Instance->GetKey() : "";
		}

		String GetDescription()
		{
			return m_Instance ? m_Instance->GetDescription() : "";
		}

		uint32 GetMinValue()
		{
			return m_Instance ? m_Instance->GetMinValue() : 0;
		}

		uint32 GetMaxValue()
		{
			return m_Instance ? m_Instance->GetMaxValue() : 0;
		}

	};


	template<>
	class ConsoleVar< int32 >
	{

	private:

		std::shared_ptr< ConsoleVarInstance< int32 > > m_Instance;
		int32 m_Default;

	public:

		ConsoleVar() = delete;
		ConsoleVar( const ConsoleVar& ) = delete;
		ConsoleVar( ConsoleVar&& ) = delete;

		ConsoleVar( const String& inKey, const String& inDescription, int32 inDefault, int32 inMin, int32 inMax,
					std::function< void( int32 ) > inCallback = nullptr, const std::string& inThread = THREAD_POOL )
			: m_Default( inDefault )
		{
			auto newVar = std::make_shared< ConsoleVarInstance< int32 > >( inKey, inDescription, inDefault, inMin, inMax, inCallback, inThread );
			if( Console::CreateVar( newVar ) )
			{
				m_Instance = newVar;
			}
		}

		~ConsoleVar()
		{
			m_Instance.reset();
		}

		bool IsValid() const
		{
			return m_Instance ? true : false;
		}

		int32 GetValue()
		{
			return m_Instance ? m_Instance->GetValue() : m_Default;
		}

		int32 GetDefault()
		{
			return m_Default;
		}

		bool SetValue( int32 inValue )
		{
			return m_Instance ? m_Instance->SetValue( inValue, true ) : false;
		}

		String GetKey()
		{
			return m_Instance ? m_Instance->GetKey() : "";
		}

		String GetDescription()
		{
			return m_Instance ? m_Instance->GetDescription() : "";
		}

		int32 GetMinValue()
		{
			return m_Instance ? m_Instance->GetMinValue() : 0;
		}

		int32 GetMaxValue()
		{
			return m_Instance ? m_Instance->GetMaxValue() : 0;
		}
		
	};

	
	template<>
	class ConsoleVar< String >
	{

	private:

		std::shared_ptr< ConsoleVarInstance< String > > m_Instance;
		String m_Default;

	public:

		ConsoleVar() = delete;
		ConsoleVar( const ConsoleVar& ) = delete;
		ConsoleVar( ConsoleVar&& ) = delete;

		ConsoleVar( const String& inKey, const String& inDescription, const String& inDefault,
					std::function< void( const String& ) > inCallback = nullptr, const std::string& inThread = THREAD_POOL )
			: m_Default( inDefault )
		{
			auto newVar = std::make_shared< ConsoleVarInstance< String > >( inKey, inDescription, inDefault, inCallback, inThread );
			if( Console::CreateVar( newVar ) )
			{
				m_Instance = newVar;
			}
		}

		~ConsoleVar()
		{
			m_Instance.reset();
		}

		bool IsValid() const
		{
			return m_Instance ? true : false;
		}

		String GetValue()
		{
			return m_Instance ? m_Instance->GetValue() : m_Default;
		}

		bool SetValue( const String& inStr )
		{
			HYPERION_VERIFY_BASICSTR( inStr );

			return m_Instance ? m_Instance->SetValue( inStr, true ) : false;
		}

		String GetKey()
		{
			return m_Instance ? m_Instance->GetKey() : "";
		}

		String GetDescription()
		{
			return m_Instance ? m_Instance->GetDescription() : "";
		}

	};
}