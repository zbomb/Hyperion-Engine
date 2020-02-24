/*==================================================================================================
	Hyperion Engine
	Source/Material/MaterialFactory.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Material/MaterialInstance.h"
#include "Hyperion/Material/MaterialAsset.h"


namespace Hyperion
{

	MaterialInstance::MaterialInstance()
		: bIsDynamic( false )
	{

	}


	MaterialInstance::MaterialInstance( const AssetRef< MaterialAsset >& inAsset, bool bDynamic )
		: bIsDynamic( bDynamic )
	{

	}


	float MaterialInstance::GetFloat( const String& inKey, float inDefault )
	{
		// Find entry in map, validate type
		auto entry = m_Parameters.find( inKey );
		if( entry == m_Parameters.end() || !entry->second.has_value() ||
			entry->second.type() != typeid( float ) )
		{
			return inDefault;
		}

		// Cast to desired type
		float value = inDefault;
		try
		{
			value = std::any_cast< float >( entry->second );
		}
		catch( std::bad_any_cast e )
		{
			(void) ( e ); // Do nothing...
		}

		return inDefault;
	}

	int32 MaterialInstance::GetInt( const String& inKey, int32 inDefault )
	{
		// Find entry in map, validate type
		auto entry = m_Parameters.find( inKey );
		if( entry == m_Parameters.end() || !entry->second.has_value() ||
			entry->second.type() != typeid( int32 ) )
		{
			return inDefault;
		}

		// Cast to desired type
		int32 value = inDefault;
		try
		{
			value = std::any_cast< int32 >( entry->second );
		}
		catch( std::bad_any_cast e )
		{
			(void) ( e ); // Do nothing...
		}

		return value;
	}

	bool MaterialInstance::GetBool( const String& inKey, bool inDefault )
	{
		// Find entry in map, we also will accept int types
		auto entry = m_Parameters.find( inKey );
		if( entry == m_Parameters.end() || !entry->second.has_value() )
		{
			return inDefault;
		}

		// Validate the type
		const auto& t = entry->second.type();
		if( t == typeid( bool ) )
		{
			try
			{
				return std::any_cast< bool >( entry->second );
			}
			catch( std::bad_any_cast e )
			{
				(void) ( e );
				return inDefault;
			}
		}
		else if( t == typeid( int32 ) )
		{
			try
			{
				int32 val = std::any_cast< int32 >( entry->second );
				if( val == 0 ) return false;
				else if( val == 1 ) return true;
				else return inDefault;
			}
			catch( std::bad_any_cast e )
			{
				(void) ( e );
				return inDefault;
			}
		}

		return inDefault;
	}


	AssetRef< TextureAsset > MaterialInstance::GetTexture( const String& inKey )
	{
		// Find entry in map
		auto entry = m_Parameters.find( inKey );
		if( entry == m_Parameters.end() || !entry->second.has_value() ||
			entry->second.type() != typeid( AssetRef< TextureAsset > ) )
		{
			return nullptr;
		}

		try
		{
			return std::any_cast<AssetRef< TextureAsset >>( entry->second );
		}
		catch( std::bad_any_cast e )
		{
			(void) ( e );
			return nullptr;
		}
	}


	bool MaterialInstance::SetValue( const String& inKey, float inValue )
	{
		// If were static, we cant modify!
		if( !bIsDynamic )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to set value in a static material!" );
			return false;
		}

		// Set value in map.. should we throw up an error if the type changes?
		auto entry = m_Parameters.find( inKey );
		if( entry != m_Parameters.end() && entry->second.has_value() )
		{
			if( entry->second.type() != typeid( float ) )
			{
				Console::WriteLine( "[WARNING] MaterialInstance: Attempt to change parameter '", inKey, "' type to float!" );
				return false;
			}
			else
			{
				entry->second = inValue;
				return true;
			}
		}

		m_Parameters.emplace( inKey, inValue );
		return true;
	}


	bool MaterialInstance::SetValue( const String& inKey, int32 inValue )
	{
		// If were static, we cant modify!
		if( !bIsDynamic )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to set value in a static material!" );
			return false;
		}

		// Set value in map.. should we throw up an error if the type changes?
		auto entry = m_Parameters.find( inKey );
		if( entry != m_Parameters.end() && entry->second.has_value() )
		{
			if( entry->second.type() != typeid( int32 ) )
			{
				Console::WriteLine( "[WARNING] MaterialInstance: Attempt to change parameter '", inKey, "' type to int!" );
				return false;
			}
			else
			{
				entry->second = inValue;
				return true;
			}
		}

		m_Parameters.emplace( inKey, inValue );
		return true;
	}


	bool MaterialInstance::SetValue( const String& inKey, bool inValue )
	{
		// If were static, we cant modify!
		if( !bIsDynamic )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to set value in a static material!" );
			return false;
		}

		// Set value in map.. should we throw up an error if the type changes?
		auto entry = m_Parameters.find( inKey );
		if( entry != m_Parameters.end() && entry->second.has_value() )
		{
			if( entry->second.type() != typeid( bool ) )
			{
				Console::WriteLine( "[WARNING] MaterialInstance: Attempt to change parameter '", inKey, "' type to bool!" );
				return false;
			}
			else
			{
				entry->second = inValue;
				return true;
			}
		}

		m_Parameters.emplace( inKey, inValue );
		return true;
	}


	bool MaterialInstance::SetValue( const String& inKey, const AssetRef< TextureAsset >& inValue )
	{
		// If were static, we cant modify!
		if( !bIsDynamic )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to set texture in a static material!" );
			return false;
		}

		if( !inValue )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to set null texture in material! (Use MaterialInstance::ClearValue instead)" );
			return false;
		}

		// Set value in map.. should we throw up an error if the type changes?
		auto entry = m_Parameters.find( inKey );
		if( entry != m_Parameters.end() && entry->second.has_value() )
		{
			if( entry->second.type() != typeid( AssetRef< TextureAsset > ) )
			{
				Console::WriteLine( "[WARNING] MaterialInstance: Attempt to change parameter '", inKey, "' type to texture!" );
				return false;
			}
			else
			{
				// Remove existing value from the texture list
				AssetRef< TextureAsset > existingAsset = nullptr;
				try
				{
					existingAsset = std::any_cast<AssetRef< TextureAsset >>( entry->second );
				}
				catch( std::bad_any_cast )
				{}

				if( existingAsset )
				{
					for( auto It = m_TextureList.begin(); It != m_TextureList.end(); )
					{
						if( *It == existingAsset )
						{
							It = m_TextureList.erase( It );
						}
						else
						{
							It++;
						}
					}
				}

				entry->second = inValue;
				return true;
			}
		}

		// Insert new texture asset into texture list
		m_TextureList.push_back( inValue );

		m_Parameters.emplace( inKey, inValue );
		return true;
	}


	bool MaterialInstance::ClearValue( const String& inKey )
	{
		if( !bIsDynamic )
		{
			Console::WriteLine( "[WARNING] MaterialInstance: Attempt to clear value from a static material!" );
			return false;
		}

		return m_Parameters.erase( inKey );
	}


}
