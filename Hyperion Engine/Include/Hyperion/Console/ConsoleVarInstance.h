/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Console/ConsoleVarInstance.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Ints.h"
#include "Hyperion/Core/String.h"

#include <shared_mutex>
#include <functional>


namespace Hyperion
{
	
	class ConsoleVarInstanceBase
	{

	public:

		virtual String GetDescription() const = 0;
		virtual String GetKey() const = 0;
		virtual String GetValueAsString() = 0;
		virtual bool SetValueFromString( const String&, bool bCallback ) = 0;

	};


	template< typename T >
	class ConsoleVarInstance : public ConsoleVarInstanceBase
	{

	public:

		ConsoleVarInstance() = delete;

	};

	template<>
	class ConsoleVarInstance< uint32 > : public ConsoleVarInstanceBase
	{

	private:

		std::shared_mutex m_Mutex;
		uint32 m_Value;
		uint32 m_Min;
		uint32 m_Max;
		String m_Key;
		String m_Description;
		std::function< void( uint32 ) > m_Callback;
		std::string m_CallbackThread;

	public:

		ConsoleVarInstance() = delete;
		ConsoleVarInstance( const String& inKey, const String& inDescription, uint32 inDefault, uint32 inMin, uint32 inMax,
							std::function< void( uint32 ) > inCallback, const std::string& callbackThread = THREAD_POOL );

		inline String GetKey() const final { return m_Key; }
		inline String GetDescription() const final { return m_Description; }
		
		String GetValueAsString() final;

		bool SetValueFromString( const String& inStr, bool bCallback ) final;

		
		bool SetValue( uint32 inValue, bool bCallback );

		uint32 GetValue();

		inline uint32 GetMinValue() const { return m_Min; }
		inline uint32 GetMaxValue() const { return m_Max; }

	};


	template<>
	class ConsoleVarInstance< int32 > : public ConsoleVarInstanceBase
	{

	private:

		std::shared_mutex m_Mutex;
		int32 m_Value;
		int32 m_Min;
		int32 m_Max;
		String m_Key;
		String m_Description;
		std::function< void( uint32 ) > m_Callback;
		std::string m_CallbackThread;

	public:

		ConsoleVarInstance() = delete;
		ConsoleVarInstance( const String& inKey, const String& inDescription, int32 inDefault, int32 inMin, int32 inMax,
							std::function< void( int32 ) > inCallback, const std::string& callbackThread = THREAD_POOL );

		inline String GetKey() const final { return m_Key; }
		inline String GetDescription() const final { return m_Description; }

		String GetValueAsString() final;

		bool SetValueFromString( const String& inStr, bool bCallback ) final;


		bool SetValue( int32 inValue, bool bCallback );

		int32 GetValue();

		inline int32 GetMinValue() const { return m_Min; }
		inline int32 GetMaxValue() const { return m_Max; }

	};


	template<>
	class ConsoleVarInstance< String > : public ConsoleVarInstanceBase
	{

	private:

		std::shared_mutex m_Mutex;
		String m_Value;
		std::function< void( const String& ) > m_Callback;
		std::string m_CallbackThread;
		String m_Key;
		String m_Description;

	public:

		ConsoleVarInstance() = delete;
		ConsoleVarInstance( const String& inKey, const String& inDescription, const String& inDefault, std::function< void( const String& ) > inCallback, const std::string& inCallbackThread = THREAD_POOL );

		inline String GetKey() const final { return m_Key; }
		inline String GetDescription() const final { return m_Description; }

		String GetValueAsString() final;

		bool SetValueFromString( const String& inStr, bool bCallback ) final;

		String GetValue();

		bool SetValue( const String& inStr, bool bCallback );

	};

	/*
		TODO: ConsoleVarInstance< float > & ConsoleVarInstance< bool > 
	*/

}